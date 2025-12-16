#include "server/PacketSender.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "server/Network.hpp"
#include "server/PacketBuffer.hpp"

namespace server {

PacketSender::PacketSender(
    ClientConnectionManager &connection_manager, Network &network)
    : connection_manager_(connection_manager), network_(network) {}

void PacketSender::SendConnectAck(ClientConnection &client,
    network::ConnectAckPacket::Status status, uint8_t assigned_player_id,
    uint16_t udp_port) {
    network::ConnectAckPacket ack;
    ack.player_id = network::PlayerId{assigned_player_id};
    ack.status = status;
    ack.udp_port = udp_port;

    network::PacketBuffer buffer;
    ack.Serialize(buffer);
    const auto &data = buffer.Data();
    auto data_copy = std::make_shared<std::vector<uint8_t>>(data);

    client.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
        [data_copy, assigned_player_id, status, udp_port](
            boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending CONNECT_ACK: " << ec.message()
                          << std::endl;
            } else {
                std::cout << "Sent CONNECT_ACK: PlayerId="
                          << static_cast<int>(assigned_player_id)
                          << ", Status=" << static_cast<int>(status)
                          << ", UdpPort=" << udp_port << std::endl;
            }
        });
}

void PacketSender::SendGameStart() {
    // Send to all authenticated players
    // Note: We need non-const access to use tcp_socket_.async_send()

    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0) {
            continue;  // Skip unauthenticated clients
        }

        // Build GAME_START packet per-client so we can include the
        // controlled entity id specific to that player.
        network::GameStartPacket game_start;
        // Use the player's assigned id as the controlled entity network id
        game_start.controlled_entity_id =
            network::EntityId{static_cast<uint32_t>(client_ref.player_id_)};

        network::PacketBuffer buffer;
        game_start.Serialize(buffer);
        const auto &data = buffer.Data();

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);

        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, client_id, player_id = client_ref.player_id_](
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

void SendSnapshotPlayerState(int tick, network::EntityState entity_state,
    network::PacketBuffer &buffer) {
    // Serialize entity state into the payload
    // Write WORLD_SNAPSHOT header (opcode 0x20 with entity data as payload)
    network::CommonHeader header(
        static_cast<uint8_t>(network::PacketType::WorldSnapshot),
        18,    // payload_size = 18 bytes (one EntityState with health)
        tick,  // tick_id = 0
        0,     // packet_index = 0
        1,     // packet_count = 1
        0);    // entity_type = 0 (Player)
    buffer.WriteHeader(header);

    // Serialize entity state into the payload
    entity_state.Serialize(buffer, network::EntityState::EntityType::Player);
}

void SendSnapshotProjectileState(int tick, network::EntityState entity_state,
    network::PacketBuffer &buffer) {
    // Serialize entity state into the payload
    // Write WORLD_SNAPSHOT header (opcode 0x20 with entity data as payload)
    network::CommonHeader header(
        static_cast<uint8_t>(network::PacketType::WorldSnapshot),
        17,    // payload_size = 17 bytes (EntityState for projectile)
        tick,  // tick_id = 0
        0,     // packet_index = 0
        1,     // packet_count = 1
        2);    // entity_type = 2 (Projectile)
    buffer.WriteHeader(header);

    // Serialize entity state into the payload
    entity_state.Serialize(
        buffer, network::EntityState::EntityType::Projectile);
}

void SendSnapshotEnemyState(int tick, network::EntityState entity_state,
    network::PacketBuffer &buffer) {
    // Serialize entity state into the payload
    // Write WORLD_SNAPSHOT header (opcode 0x20 with entity data as payload)
    network::CommonHeader header(
        static_cast<uint8_t>(network::PacketType::WorldSnapshot),
        20,    // payload_size = 20 bytes (one EntityState with animation and
               // health)
        tick,  // tick_id = 0
        0,     // packet_index = 0
        1,     // packet_count = 1
        1);    // entity_type = 1 (Enemy)
    buffer.WriteHeader(header);

    // Serialize entity state into the payload
    entity_state.Serialize(buffer, network::EntityState::EntityType::Enemy);
}

void PacketSender::SendSnapshot(network::EntityState entity_state, int tick) {
    // Send entity state snapshot to all authenticated players via UDP
    network::PacketBuffer buffer;

    if (entity_state.entity_type == 0) {  // PLAYER
        SendSnapshotPlayerState(tick, entity_state, buffer);
    } else if (entity_state.entity_type == 1) {  // ENEMY
        SendSnapshotEnemyState(tick, entity_state, buffer);
    } else if (entity_state.entity_type == 2) {  // PROJECTILE
        SendSnapshotProjectileState(tick, entity_state, buffer);
    } else {
        std::cerr << "Unknown entity type for snapshot: "
                  << static_cast<int>(entity_state.entity_type) << std::endl;
        return;
    }

    // Send to all authenticated players
    const auto &data = buffer.Data();
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients
        // Use UDP for snapshots (unreliable but fast gameplay data)
        std::array<uint8_t, Network::MAX_UDP_PACKET_SIZE> udp_buffer{};
        std::copy(data.begin(), data.end(), udp_buffer.begin());
        network_.SendUdp(udp_buffer, data.size(), client_ref.udp_endpoint_);
    }
}

void PacketSender::SendLobbyStatus() {
    // Count authenticated players and ready players
    uint8_t connected_count = 0;
    uint8_t ready_count = 0;

    auto &clients = connection_manager_.GetClients();
    for (const auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ != 0) {
            connected_count++;
            if (client_ref.ready_) {
                ready_count++;
            }
        }
    }

    // Build LOBBY_STATUS packet
    network::LobbyStatusPacket lobby_status;
    lobby_status.connected_count = connected_count;
    lobby_status.ready_count = ready_count;
    lobby_status.max_players = connection_manager_.GetMaxClients();
    lobby_status.reserved = 0;

    network::PacketBuffer buffer;
    lobby_status.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send to all authenticated players via TCP
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0) {
            continue;  // Skip unauthenticated clients
        }

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, connected_count, ready_count](
                boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Error sending LOBBY_STATUS: " << ec.message()
                              << std::endl;
                }
            });
    }

    std::cout << "Sent LOBBY_STATUS: " << static_cast<int>(connected_count)
              << "/" << static_cast<int>(connection_manager_.GetMaxClients())
              << " players, " << static_cast<int>(ready_count) << " ready"
              << std::endl;
}

}  // namespace server
