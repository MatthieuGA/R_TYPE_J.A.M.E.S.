#include "server/Server.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/Components.hpp"
#include "server/PacketBuffer.hpp"
#include "server/Packets.hpp"

namespace server {

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(std::make_unique<Network>(config, io_context)),
      registry_(),
      tick_timer_(io_context),
      running_(false),
      next_player_id_(1) {}

Server::~Server() {
    running_ = false;
}

void Server::Initialize() {
    std::cout << "Initializing server..." << std::endl;
    RegisterComponents();
    RegisterSystems();

    // Register TCP accept callback
    network_->GetTcp().SetAcceptCallback(
        [this](boost::asio::ip::tcp::socket socket) {
            HandleTcpAccept(std::move(socket));
        });

    std::cout << "Server initialized successfully" << std::endl;
    std::cout << "Ready to accept TCP connections (max "
              << static_cast<int>(MAX_CLIENTS) << " players)" << std::endl;
}

void Server::RegisterComponents() {
    registry_.RegisterComponent<Component::Position>();
    registry_.RegisterComponent<Component::Velocity>();
    registry_.RegisterComponent<Component::Health>();
    registry_.RegisterComponent<Component::NetworkId>();
    registry_.RegisterComponent<Component::Player>();
    registry_.RegisterComponent<Component::Enemy>();

    std::cout << "Registered all components" << std::endl;
}

void Server::RegisterSystems() {
    registry_.AddSystem<Engine::sparse_array<Component::Position>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Position> &positions,
            Engine::sparse_array<Component::Velocity> &velocities) {
            for (size_t i = 0; i < positions.size() && i < velocities.size();
                ++i) {
                if (positions.has(i) && velocities.has(i)) {
                    auto &pos = positions[i];
                    auto &vel = velocities[i];
                    pos->x += vel->vx;
                    pos->y += vel->vy;
                }
            }
        });

    std::cout << "Registered all systems" << std::endl;
}

void Server::Start() {
    std::cout << "Starting server..." << std::endl;
    running_ = true;
    SetupGameTick();
}

void Server::SetupGameTick() {
    if (!running_) {
        return;
    }

    tick_timer_.expires_after(std::chrono::milliseconds(TICK_RATE_MS));
    tick_timer_.async_wait([this](const boost::system::error_code &ec) {
        if (!ec && running_) {
            Update();
            SetupGameTick();
        }
    });
}

void Server::Update() {
    registry_.run_systems();

    // TODO(someone): Process network messages from the SPSC queue
    // TODO(someone): Send state updates to clients
}

Engine::registry &Server::GetRegistry() {
    return registry_;
}

// ============================================================================
// TCP Connection Handlers
// ============================================================================

void Server::HandleTcpAccept(boost::asio::ip::tcp::socket socket) {
    // Allocate buffer on heap (lambda must own it for async lifetime)
    auto buffer = std::make_shared<std::vector<uint8_t>>(44);  // 12 + 32 bytes

    // Move socket into shared_ptr before calling async_receive
    // (can't call method on socket after it's moved into lambda capture)
    auto socket_ptr =
        std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));

    // Start async receive for CONNECT_REQ packet
    socket_ptr->async_receive(boost::asio::buffer(*buffer),
        [this, buffer, socket_ptr](
            boost::system::error_code ec, std::size_t bytes_read) {
            if (ec) {
                std::cerr << "Error reading CONNECT_REQ: " << ec.message()
                          << std::endl;
                return;
            }

            // CONNECT_REQ must be exactly 44 bytes (12 header + 32 username)
            if (bytes_read < 44) {
                std::cerr << "Incomplete CONNECT_REQ: expected 44 bytes, got "
                          << bytes_read << std::endl;
                return;
            }

            // Parse header
            network::PacketBuffer packet_buffer(*buffer);
            network::CommonHeader header = packet_buffer.ReadHeader();

            if (header.op_code !=
                static_cast<uint8_t>(network::PacketType::ConnectReq)) {
                std::cerr << "Expected CONNECT_REQ (0x01), got opcode 0x"
                          << std::hex << static_cast<int>(header.op_code)
                          << std::dec << std::endl;
                return;
            }

            // Extract payload (username)
            std::vector<uint8_t> payload(
                buffer->begin() + 12,  // Skip 12-byte header
                buffer->begin() + 12 +
                    32);  // 32-byte username null-terminated

            HandleConnectReq(*socket_ptr, payload);
        });
}

void Server::HandleConnectReq(boost::asio::ip::tcp::socket &socket,
    const std::vector<uint8_t> &payload) {
    // Deserialize CONNECT_REQ
    network::PacketBuffer buffer(payload);
    network::ConnectReqPacket req =
        network::ConnectReqPacket::Deserialize(buffer);
    std::string username = req.GetUsername();

    std::cout << "CONNECT_REQ with username: '" << username << "'"
              << std::endl;

    // Validation: Empty username
    if (username.empty() ||
        username.find_first_not_of('\0') == std::string::npos) {
        std::cerr << "Rejected: Empty username" << std::endl;
        // Keep socket alive for async_send via shared_ptr
        auto socket_ptr =
            std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));
        SendConnectAck(*socket_ptr, 0, network::ConnectAckPacket::BadUsername,
            socket_ptr);
        return;
    }

    // Validation: Username already taken
    if (IsUsernameTaken(username)) {
        std::cerr << "Rejected: Username '" << username << "' already taken"
                  << std::endl;
        // Keep socket alive for async_send via shared_ptr
        auto socket_ptr =
            std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));
        SendConnectAck(*socket_ptr, 0, network::ConnectAckPacket::BadUsername,
            socket_ptr);
        return;
    }

    // Validation: Server full
    if (clients_.size() >= MAX_CLIENTS) {
        std::cerr << "Rejected: Server full (" << clients_.size() << "/"
                  << static_cast<int>(MAX_CLIENTS) << " players)" << std::endl;
        // Keep socket alive for async_send via shared_ptr
        auto socket_ptr =
            std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));
        SendConnectAck(
            *socket_ptr, 0, network::ConnectAckPacket::ServerFull, socket_ptr);
        return;
    }

    // Assign PlayerId and add to clients map
    uint8_t player_id = AssignPlayerId();

    // Create ClientConnection and move socket ownership
    ClientConnection connection(player_id, std::move(socket));
    connection.username = username;

    std::cout << "Player connected: '" << username << "' assigned ID "
              << static_cast<int>(player_id) << std::endl;

    // Send CONNECT_ACK (must send before moving connection into map)
    SendConnectAck(
        connection.tcp_socket, player_id, network::ConnectAckPacket::OK);

    // Transfer ownership to clients_ map
    clients_.emplace(player_id, std::move(connection));

    // Start monitoring for disconnect
    MonitorClientDisconnect(player_id);
}

void Server::SendConnectAck(boost::asio::ip::tcp::socket &socket,
    uint8_t player_id, network::ConnectAckPacket::Status status,
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_keeper) {
    network::ConnectAckPacket ack;
    ack.player_id = network::PlayerId{player_id};
    ack.status = status;
    ack.reserved = {0, 0};

    // Serialize packet
    network::PacketBuffer buffer;
    ack.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send async (use shared_ptr to keep data alive)
    auto data_copy = std::make_shared<std::vector<uint8_t>>(data);

    socket.async_send(boost::asio::buffer(*data_copy),
        [data_copy, socket_keeper, player_id, status](
            boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending CONNECT_ACK to player "
                          << static_cast<int>(player_id) << ": "
                          << ec.message() << std::endl;
            } else {
                std::cout << "Sent CONNECT_ACK: PlayerId="
                          << static_cast<int>(player_id)
                          << ", Status=" << static_cast<int>(status)
                          << std::endl;
            }
            // socket_keeper automatically cleans up socket when this lambda is
            // destroyed
        });
}

uint8_t Server::AssignPlayerId() {
    uint8_t id = next_player_id_++;

    // Handle wraparound (skip 0, which is reserved for "no player")
    if (next_player_id_ == 0) {
        next_player_id_ = 1;
    }

    // Ensure uniqueness (in case of wraparound collision)
    while (clients_.find(id) != clients_.end()) {
        id = next_player_id_++;
        if (next_player_id_ == 0) {
            next_player_id_ = 1;
        }
    }

    return id;
}

void Server::RemoveClient(uint8_t player_id) {
    auto it = clients_.find(player_id);
    if (it == clients_.end()) {
        std::cerr << "removeClient: Player " << static_cast<int>(player_id)
                  << " not found" << std::endl;
        return;
    }

    std::cout << "Player " << static_cast<int>(player_id) << " ('"
              << it->second.username << "') disconnected" << std::endl;

    // TCP socket closes automatically (object is destroyed)
    clients_.erase(it);
}

bool Server::IsUsernameTaken(const std::string &username) const {
    return std::any_of(
        clients_.begin(), clients_.end(), [&username](const auto &pair) {
            return pair.second.username == username;
        });
}

void Server::MonitorClientDisconnect(uint8_t player_id) {
    auto it = clients_.find(player_id);
    if (it == clients_.end()) {
        return;  // Client already disconnected
    }

    // Allocate a small buffer for detecting socket closure
    auto buffer = std::make_shared<std::vector<uint8_t>>(1);

    // Start async read - any data or EOF will trigger the callback
    it->second.tcp_socket.async_receive(boost::asio::buffer(*buffer),
        [this, buffer, player_id](
            boost::system::error_code ec, std::size_t bytes_read) {
            if (ec) {
                // Socket closed or error - remove client
                std::cout << "Client " << static_cast<int>(player_id)
                          << " disconnected: " << ec.message() << std::endl;
                RemoveClient(player_id);
            } else if (bytes_read > 0) {
                // Received unexpected data - for now, just continue monitoring
                // In a full implementation, this would handle ongoing messages
                std::cout << "Received " << bytes_read << " bytes from client "
                          << static_cast<int>(player_id)
                          << " (unexpected during lobby)" << std::endl;
                MonitorClientDisconnect(player_id);  // Continue monitoring
            } else {
                // EOF without error - client disconnected gracefully
                std::cout << "Client " << static_cast<int>(player_id)
                          << " disconnected gracefully" << std::endl;
                RemoveClient(player_id);
            }
        });
}

}  // namespace server
