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

static std::string trim(
    const std::string &str, const std::string &whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

namespace server {

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(std::make_unique<Network>(config, io_context)),
      registry_(),
      tick_timer_(io_context),
      running_(false),
      max_clients_(config.GetMaxPlayers()),
      next_client_id_(1),
      next_player_id_(1) {}

Server::~Server() {
    running_ = false;
}

void Server::Initialize() {
    std::cout << "Initializing server..." << std::endl;
    RegisterComponents();
    RegisterSystems();
    RegisterPacketHandlers();

    // Register TCP accept callback
    network_->GetTcp().SetAcceptCallback(
        [this](boost::asio::ip::tcp::socket socket) {
            HandleTcpAccept(std::move(socket));
        });

    std::cout << "Server initialized successfully" << std::endl;
    std::cout << "Ready to accept TCP connections (max "
              << static_cast<int>(max_clients_) << " players)" << std::endl;
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
    std::cout << "Starting game..." << std::endl;
    running_ = true;
    SetupGameTick();
}

void Server::Stop() {
    std::cout << "Stopping game..." << std::endl;
    running_ = false;
    tick_timer_.cancel();
    std::cout << "Game stopped" << std::endl;
}

void Server::Close() {
    std::cout << "Closing server..." << std::endl;
    Stop();
    // Close all client connections
    for (auto &[client_id, connection] : clients_) {
        std::cout << "Closing connection for client " << client_id
                  << " (Player " << static_cast<int>(connection.player_id_)
                  << ", '" << connection.username_ << "')" << std::endl;
        boost::system::error_code ec;
        if (connection.tcp_socket_.close(ec) && ec) {
            std::cerr << "Error closing socket for client " << client_id
                      << ": " << ec.message() << std::endl;
        }
    }
    clients_.clear();
    std::cout << "Server closed" << std::endl;
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
    // Assign unique internal client_id
    uint32_t client_id = AssignClientId();
    std::cout << "New TCP connection accepted, assigned client ID "
              << client_id << std::endl;

    // Create unauthenticated connection (player_id=0 until authenticated)
    // Client must send CONNECT_REQ to get a player_id assigned
    ClientConnection connection(client_id, 0, std::move(socket));

    // Add to clients map
    clients_.emplace(client_id, std::move(connection));

    // Start handling messages immediately
    HandleClientMessages(client_id);
}

uint32_t Server::AssignClientId() {
    return next_client_id_++;
}

uint8_t Server::AssignPlayerId() {
    uint8_t id = next_player_id_++;
    if (next_player_id_ == 0)
        next_player_id_ = 1;
    // Find first available player_id (check authenticated clients only)
    uint8_t attempts = 0;
    while (attempts < 255) {
        bool id_taken = false;
        for (const auto &[cid, conn] : clients_) {
            if (conn.player_id_ != 0 && conn.player_id_ == id) {
                id_taken = true;
                break;
            }
        }
        if (!id_taken)
            return id;
        id = next_player_id_++;
        if (next_player_id_ == 0)
            next_player_id_ = 1;
        attempts++;
    }
    throw std::runtime_error("All player IDs exhausted");
}

void Server::RemoveClient(uint32_t client_id) {
    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        std::cerr << "RemoveClient: Client " << client_id << " not found"
                  << std::endl;
        return;
    }
    if (it->second.player_id_ != 0) {
        std::cout << "Client " << client_id << " (Player "
                  << static_cast<int>(it->second.player_id_) << ", '"
                  << it->second.username_ << "') disconnected" << std::endl;
    } else {
        std::cout << "Client " << client_id
                  << " (unauthenticated) disconnected" << std::endl;
    }
    // TCP socket closes automatically (object is destroyed)
    clients_.erase(it);
}

bool Server::IsUsernameTaken(const std::string &username) const {
    return std::any_of(
        clients_.begin(), clients_.end(), [&username](const auto &pair) {
            return pair.second.player_id_ != 0 &&
                   pair.second.username_ == username;
        });
}

void Server::HandleClientMessages(uint32_t client_id) {
    auto it = clients_.find(client_id);
    if (it == clients_.end())
        return;  // Client already disconnected
    boost::asio::ip::tcp::socket &socket = it->second.tcp_socket_;

    // Allocate buffer for receiving TCP messages (max 1024 bytes)
    auto buffer = std::make_shared<std::vector<uint8_t>>(1024);

    // Start async read - any data or EOF will trigger the callback
    socket.async_receive(boost::asio::buffer(*buffer),
        [this, buffer, client_id](
            boost::system::error_code ec, std::size_t bytes_read) {
            if (ec) {
                // Socket closed or error - remove client
                std::cout << "Client " << client_id
                          << " disconnected: " << ec.message() << std::endl;
                RemoveClient(client_id);
            } else if (bytes_read > 0) {
                // Update last activity timestamp and get client reference
                auto it = clients_.find(client_id);
                if (it == clients_.end())
                    return;  // Client was removed
                it->second.last_activity_ = std::chrono::steady_clock::now();

                // Parse incoming TCP packet using PacketFactory
                network::PacketParseResult result =
                    network::DeserializePacket(buffer->data(), bytes_read);

                if (!result.success) {
                    std::cerr << "Failed to parse packet from client "
                              << client_id << ": " << result.error
                              << std::endl;
                } else {
                    // Dispatch to appropriate handler based on packet type
                    auto packet_type = static_cast<network::PacketType>(
                        result.header.op_code);
                    auto handler_it = packet_handlers_.find(packet_type);

                    if (handler_it != packet_handlers_.end()) {
                        // Pass ClientConnection reference to handler
                        handler_it->second(it->second, result.packet);
                    } else {
                        std::cerr << "No handler registered for packet type 0x"
                                  << std::hex << static_cast<int>(packet_type)
                                  << std::dec << " from client " << client_id
                                  << std::endl;
                    }
                }
                // Continue handling messages if connection still exists
                if (clients_.find(client_id) != clients_.end())
                    HandleClientMessages(client_id);
            } else {
                // EOF without error - client disconnected gracefully
                std::cout << "Client " << client_id
                          << " disconnected gracefully" << std::endl;
                RemoveClient(client_id);
            }
        });
}

// ============================================================================
// TCP Packet Handler Registration and Handlers
// ============================================================================

void Server::RegisterPacketHandlers() {
    // Register CONNECT_REQ handler (0x01)
    packet_handlers_[network::PacketType::ConnectReq] =
        [this](
            ClientConnection &client, const network::PacketVariant &packet) {
            if (const auto *connect_packet =
                    std::get_if<network::ConnectReqPacket>(&packet)) {
                HandleConnectReq(client, *connect_packet);
            }
        };
    // Register READY_STATUS handler (0x07)
    packet_handlers_[network::PacketType::ReadyStatus] =
        [this](
            ClientConnection &client, const network::PacketVariant &packet) {
            if (const auto *ready_packet =
                    std::get_if<network::ReadyStatusPacket>(&packet)) {
                HandleReadyStatus(client, *ready_packet);
            }
        };
    // Register DISCONNECT_REQ handler (0x03)
    packet_handlers_[network::PacketType::DisconnectReq] =
        [this](
            ClientConnection &client, const network::PacketVariant &packet) {
            if (const auto *disconnect_packet =
                    std::get_if<network::DisconnectReqPacket>(&packet)) {
                HandleDisconnectReq(client, *disconnect_packet);
            }
        };
    std::cout << "Registered " << packet_handlers_.size() << " packet handlers"
              << std::endl;
}

void Server::HandleConnectReq(
    ClientConnection &client, const network::ConnectReqPacket &packet) {
    std::string username = trim(packet.GetUsername());
    std::cout << "CONNECT_REQ from client " << client.client_id_
              << " with username: '" << username << "'" << std::endl;

    // Validation: Empty username
    if (username.empty() ||
        username.find_first_not_of('\0') == std::string::npos) {
        std::cerr << "Rejected: Empty username" << std::endl;
        SendConnectAck(client, network::ConnectAckPacket::BadUsername);
        return;  // Keep connection alive for retry
    }

    // Validation: Username already taken
    if (IsUsernameTaken(username)) {
        std::cerr << "Rejected: Username '" << username << "' already taken"
                  << std::endl;
        SendConnectAck(client, network::ConnectAckPacket::BadUsername);
        return;  // Keep connection alive for retry
    }

    // Validation: Server full (count only authenticated clients)
    size_t authenticated_count = 0;
    for (const auto &[id, conn] : clients_) {
        if (conn.player_id_ != 0) {
            authenticated_count++;
        }
    }

    if (authenticated_count >= max_clients_) {
        std::cerr << "Rejected: Server full (" << authenticated_count << "/"
                  << static_cast<int>(max_clients_) << " players)"
                  << std::endl;
        SendConnectAck(client, network::ConnectAckPacket::ServerFull);
        return;  // Keep connection alive for retry
    }

    // Assign player_id to authenticate client
    uint8_t player_id = AssignPlayerId();
    client.player_id_ = player_id;
    client.username_ = username;

    std::cout << "Client " << client.client_id_ << " authenticated as Player "
              << static_cast<int>(player_id) << " ('" << username << "')"
              << std::endl;

    // Send success response with assigned player_id
    SendConnectAck(client, network::ConnectAckPacket::OK, player_id);
}

void Server::HandleReadyStatus(
    ClientConnection &client, const network::ReadyStatusPacket &packet) {
    // Only authenticated players can send READY_STATUS
    if (client.player_id_ == 0) {
        std::cerr << "READY_STATUS from unauthenticated client "
                  << client.client_id_
                  << ". Client must send CONNECT_REQ first." << std::endl;
        return;
    }

    bool is_ready = (packet.is_ready != 0);
    client.ready_ = is_ready;

    std::cout << "Player " << static_cast<int>(client.player_id_) << " ('"
              << client.username_ << "') is now "
              << (is_ready ? "READY" : "NOT READY") << std::endl;

    // Check if all authenticated players are ready
    bool all_ready = true;
    size_t connected_players = 0;
    for (const auto &[id, connection] : clients_) {
        if (connection.player_id_ == 0)
            continue;  // Skip unauthenticated connections
        connected_players++;
        if (!connection.ready_) {
            all_ready = false;
            break;
        }
    }
    if (all_ready && connected_players > 0) {
        std::cout << "All " << connected_players
                  << " players ready! Starting game..." << std::endl;
        Start();  // Start before sending packets to clients to initialize
                  // their player entity
        SendGameStart();
    }
}

void Server::HandleDisconnectReq(
    ClientConnection &client, const network::DisconnectReqPacket &packet) {
    (void)packet;  // No payload in DISCONNECT_REQ

    if (client.player_id_ != 0) {
        std::cout << "Player " << static_cast<int>(client.player_id_) << " ('"
                  << client.username_ << "') requested disconnect"
                  << std::endl;
    } else {
        std::cout << "Client " << client.client_id_
                  << " (unauthenticated) requested disconnect" << std::endl;
    }

    // Remove client (graceful shutdown)
    RemoveClient(client.client_id_);
    // TODO(future): Notify other clients about disconnection
    // (NOTIFY_DISCONNECT)
}

// ============================================================================
// TCP Packet Senders
// ============================================================================

void Server::SendConnectAck(ClientConnection &client,
    network::ConnectAckPacket::Status status, uint8_t assigned_player_id) {
    network::ConnectAckPacket ack;
    ack.player_id = network::PlayerId{assigned_player_id};
    ack.status = status;
    ack.reserved = {0, 0};

    network::PacketBuffer buffer;
    ack.Serialize(buffer);
    const auto &data = buffer.Data();
    auto data_copy = std::make_shared<std::vector<uint8_t>>(data);

    client.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
        [data_copy, assigned_player_id, status](
            boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending CONNECT_ACK: " << ec.message()
                          << std::endl;
            } else {
                std::cout << "Sent CONNECT_ACK: PlayerId="
                          << static_cast<int>(assigned_player_id)
                          << ", Status=" << static_cast<int>(status)
                          << std::endl;
            }
        });
}

void Server::SendGameStart() {
    // Send to all authenticated players
    for (auto &[client_id, client] : clients_) {
        if (client.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        network::GameStartPacket game_start;
        // TODO(Matthieu-GA): Assign actual controlled entity ID from ECS
        game_start.controlled_entity_id = network::EntityId{0};

        network::PacketBuffer buffer;
        game_start.Serialize(buffer);
        const auto &data = buffer.Data();
        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);

        client.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, client_id, player_id = client.player_id_](
                boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Error sending GAME_START to client "
                              << client_id << ": " << ec.message()
                              << std::endl;
                } else {
                    std::cout << "Sent GAME_START to Player "
                              << static_cast<int>(player_id) << std::endl;
                }
            });
    }

    std::cout << "GAME_START packet sent to all authenticated players"
              << std::endl;
}

}  // namespace server
