#include "server/PacketDispatcher.hpp"

#include <iostream>
#include <string>

#include "server/ServerMessenger.hpp"

namespace server {

PacketDispatcher::PacketDispatcher(
    ClientConnectionManager &connection_manager, ServerMessenger &messenger)
    : connection_manager_(connection_manager),
      messenger_(messenger),
      on_game_start_(nullptr) {}

void PacketDispatcher::RegisterHandlers() {
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

void PacketDispatcher::SetGameStartCallback(GameStartCallback callback) {
    on_game_start_ = callback;
}

void PacketDispatcher::Dispatch(
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

void PacketDispatcher::HandleConnectReq(
    ClientConnection &client, const network::ConnectReqPacket &packet) {
    std::string username = Trim(packet.GetUsername());
    std::cout << "CONNECT_REQ from client " << client.client_id_
              << " with username: '" << username << "'" << std::endl;

    // Validation: Empty username
    if (username.empty() ||
        username.find_first_not_of('\0') == std::string::npos) {
        std::cerr << "Rejected: Empty username" << std::endl;
        messenger_.SendConnectAck(
            client, network::ConnectAckPacket::BadUsername);
        return;  // Keep connection alive for retry
    }

    // Validation: Username already taken
    if (connection_manager_.IsUsernameTaken(username)) {
        std::cerr << "Rejected: Username '" << username << "' already taken"
                  << std::endl;
        messenger_.SendConnectAck(
            client, network::ConnectAckPacket::BadUsername);
        return;  // Keep connection alive for retry
    }

    // Validation: Server full
    if (connection_manager_.IsFull()) {
        std::cerr << "Rejected: Server full ("
                  << connection_manager_.GetAuthenticatedCount() << "/"
                  << static_cast<int>(connection_manager_.GetMaxClients())
                  << " players)" << std::endl;
        messenger_.SendConnectAck(
            client, network::ConnectAckPacket::ServerFull);
        return;  // Keep connection alive for retry
    }

    // Authenticate client (assigns player_id and username)
    uint8_t player_id =
        connection_manager_.AuthenticateClient(client.client_id_, username);

    if (player_id == 0) {
        std::cerr << "Failed to authenticate client " << client.client_id_
                  << std::endl;
        messenger_.SendConnectAck(
            client, network::ConnectAckPacket::BadUsername);
        return;
    }

    // Send success response with assigned player_id
    messenger_.SendConnectAck(
        client, network::ConnectAckPacket::OK, player_id);
}

void PacketDispatcher::HandleReadyStatus(
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
    if (connection_manager_.AllPlayersReady()) {
        size_t connected_players = connection_manager_.GetAuthenticatedCount();
        std::cout << "All " << connected_players
                  << " players ready! Starting game..." << std::endl;
        if (on_game_start_) {
            on_game_start_();  // Trigger game start via callback
        }
        messenger_.SendGameStart();
    }
}

void PacketDispatcher::HandleDisconnectReq(
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

std::string PacketDispatcher::Trim(
    const std::string &str, const std::string &whitespace) {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

}  // namespace server
