#include "server/PacketHandler.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "server/Network.hpp"
#include "server/PacketSender.hpp"

namespace server {

PacketHandler::PacketHandler(ClientConnectionManager &connection_manager,
    PacketSender &packet_sender, Network &network)
    : connection_manager_(connection_manager),
      packet_sender_(packet_sender),
      network_(network),
      on_game_start_(nullptr) {}

void PacketHandler::RegisterHandlers() {
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

void PacketHandler::SetGameStartCallback(GameStartCallback callback) {
    on_game_start_ = callback;
}

void PacketHandler::StartReceiving(uint32_t client_id) {
    HandleClientMessages(client_id);
}

void PacketHandler::HandleClientMessages(uint32_t client_id) {
    if (!connection_manager_.HasClient(client_id)) {
        return;  // Client already disconnected
    }

    ClientConnection &client = connection_manager_.GetClient(client_id);
    boost::asio::ip::tcp::socket &socket = client.tcp_socket_;

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
                connection_manager_.RemoveClient(client_id);
            } else if (bytes_read > 0) {
                // Update last activity timestamp
                if (!connection_manager_.HasClient(client_id)) {
                    return;  // Client was removed
                }

                ClientConnection &client =
                    connection_manager_.GetClient(client_id);
                client.last_activity_ = std::chrono::steady_clock::now();

                // Parse incoming TCP packet using PacketFactory
                network::PacketParseResult result =
                    network::DeserializePacket(buffer->data(), bytes_read);

                // Dispatch to handler
                Dispatch(client_id, result);

                // Continue handling messages if connection still exists
                if (connection_manager_.HasClient(client_id)) {
                    HandleClientMessages(client_id);
                }
            } else {
                // EOF without error - client disconnected gracefully
                std::cout << "Client " << client_id
                          << " disconnected gracefully" << std::endl;
                connection_manager_.RemoveClient(client_id);
            }
        });
}

void PacketHandler::Dispatch(
    uint32_t client_id, const network::PacketParseResult &result) {
    if (!result.success) {
        std::cerr << "Failed to parse packet from client " << client_id << ": "
                  << result.error << std::endl;
        return;
    }

    // Get client reference from connection manager
    if (!connection_manager_.HasClient(client_id)) {
        std::cerr << "Dispatch: Client " << client_id << " not found"
                  << std::endl;
        return;
    }

    ClientConnection &client = connection_manager_.GetClient(client_id);

    // Dispatch to appropriate handler based on packet type
    auto packet_type = static_cast<network::PacketType>(result.header.op_code);
    auto handler_it = packet_handlers_.find(packet_type);

    if (handler_it != packet_handlers_.end()) {
        handler_it->second(client, result.packet);
    } else {
        std::cerr << "No handler registered for packet type 0x" << std::hex
                  << static_cast<int>(packet_type) << std::dec
                  << " from client " << client_id << std::endl;
    }
}

void PacketHandler::HandleConnectReq(
    ClientConnection &client, const network::ConnectReqPacket &packet) {
    std::string username = Trim(packet.GetUsername());
    std::cout << "CONNECT_REQ from client " << client.client_id_
              << " with username: '" << username << "'" << std::endl;

    // Validation: Empty username
    if (username.empty() ||
        username.find_first_not_of('\0') == std::string::npos) {
        std::cerr << "Rejected: Empty username" << std::endl;
        packet_sender_.SendConnectAck(client,
            network::ConnectAckPacket::BadUsername, 0, network_.GetUdpPort());
        return;  // Keep connection alive for retry
    }

    // Validation: Username already taken
    if (connection_manager_.IsUsernameTaken(username)) {
        std::cerr << "Rejected: Username '" << username << "' already taken"
                  << std::endl;
        packet_sender_.SendConnectAck(client,
            network::ConnectAckPacket::BadUsername, 0, network_.GetUdpPort());
        return;  // Keep connection alive for retry
    }

    // Validation: Server full
    if (connection_manager_.IsFull()) {
        std::cerr << "Rejected: Server full ("
                  << connection_manager_.GetAuthenticatedCount() << "/"
                  << static_cast<int>(connection_manager_.GetMaxClients())
                  << " players)" << std::endl;
        packet_sender_.SendConnectAck(client,
            network::ConnectAckPacket::ServerFull, 0, network_.GetUdpPort());
        return;  // Keep connection alive for retry
    }

    // Authenticate client (assigns player_id and username)
    uint8_t player_id =
        connection_manager_.AuthenticateClient(client.client_id_, username);

    if (player_id == 0) {
        std::cerr << "Failed to authenticate client " << client.client_id_
                  << std::endl;
        packet_sender_.SendConnectAck(client,
            network::ConnectAckPacket::BadUsername, 0, network_.GetUdpPort());
        return;
    }

    // Send success response with assigned player_id and server's UDP port
    packet_sender_.SendConnectAck(client, network::ConnectAckPacket::OK,
        player_id, network_.GetUdpPort());
}

void PacketHandler::HandleReadyStatus(
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

    // Count how many players are ready
    size_t ready_count = 0;
    size_t total_authenticated = 0;
    for (const auto &[id, conn] : connection_manager_.GetClients()) {
        if (conn.player_id_ != 0) {
            total_authenticated++;
            if (conn.ready_) {
                ready_count++;
            }
        }
    }
    std::cout << "Players ready: " << ready_count << "/" << total_authenticated
              << std::endl;

    // Check if all authenticated players are ready
    if (connection_manager_.AllPlayersReady()) {
        size_t connected_players = connection_manager_.GetAuthenticatedCount();
        std::cout << "All " << connected_players
                  << " players ready! Starting game..." << std::endl;
        if (on_game_start_) {
            on_game_start_();  // Trigger game start via callback
        }
        packet_sender_.SendGameStart();
    }
}

void PacketHandler::HandleDisconnectReq(
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
    connection_manager_.RemoveClient(client.client_id_);
    // TODO(future): Notify other clients about disconnection
    // (NOTIFY_DISCONNECT)
}

std::string PacketHandler::Trim(
    const std::string &str, const std::string &whitespace) {
    const auto str_begin = str.find_first_not_of(whitespace);
    if (str_begin == std::string::npos)
        return "";  // no content
    const auto str_end = str.find_last_not_of(whitespace);
    const auto str_range = str_end - str_begin + 1;
    return str.substr(str_begin, str_range);
}

}  // namespace server
