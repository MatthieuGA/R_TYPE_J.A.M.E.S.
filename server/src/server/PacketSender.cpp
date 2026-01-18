#include "server/PacketSender.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "server/Network.hpp"
#include "server/PacketBuffer.hpp"

namespace server {

PacketSender::PacketSender(
    ClientConnectionManager &connection_manager, Network &network)
    : connection_manager_(connection_manager), network_(network) {}

void PacketSender::SendConnectAck(ClientConnection &client,
    network::ConnectAckPacket::Status status, uint8_t assigned_player_id) {
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

    network::ConnectAckPacket ack;
    ack.player_id = network::PlayerId{assigned_player_id};
    ack.status = status;
    ack.connected_players = connected_count;
    ack.ready_players = ready_count;
    ack.max_players = connection_manager_.GetMaxClients();
    ack.min_players = 1;  // Minimum 1 player to start
    ack.reserved = 0;

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

void PacketSender::SendGameEnd(uint8_t winning_player_id) {
    // Build GAME_END packet
    network::GameEndPacket game_end;
    game_end.winning_player_id = network::PlayerId{winning_player_id};
    game_end.reserved = {0, 0, 0};

    network::PacketBuffer buffer;
    game_end.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send to all authenticated players via TCP
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0) {
            continue;  // Skip unauthenticated clients
        }

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, player_id = client_ref.player_id_](
                boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Error sending GAME_END: " << ec.message()
                              << std::endl;
                } else {
                    std::cout << "Sent GAME_END to Player "
                              << static_cast<int>(player_id) << std::endl;
                }
            });
    }

    std::cout << "GAME_END packet sent to all authenticated players "
              << "(winning_player_id=" << static_cast<int>(winning_player_id)
              << ")" << std::endl;
}

void PacketSender::SendNotifyConnect(
    uint8_t player_id, const std::string &username) {
    // Build NOTIFY_CONNECT packet
    network::NotifyConnectPacket notify_connect(
        network::PlayerId{player_id}, username);

    network::PacketBuffer buffer;
    notify_connect.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send to all authenticated players via TCP (except the new player)
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        if (client_ref.player_id_ == player_id)
            continue;  // Don't notify the new player about themselves

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, player_id, username](
                boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_CONNECT for player "
                              << static_cast<int>(player_id) << " ('"
                              << username << "'): " << ec.message()
                              << std::endl;
                }
            });
    }

    std::cout << "NOTIFY_CONNECT packet sent to all authenticated players "
              << "(player_id=" << static_cast<int>(player_id) << ", username='"
              << username << "')" << std::endl;
}

void PacketSender::SendNotifyReady(uint8_t player_id, bool is_ready) {
    // Build NOTIFY_READY packet
    network::NotifyReadyPacket notify_ready(
        network::PlayerId{player_id}, is_ready);

    network::PacketBuffer buffer;
    notify_ready.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send to all authenticated players via TCP
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, player_id, is_ready](
                boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_READY for player "
                              << static_cast<int>(player_id)
                              << " (ready=" << is_ready
                              << "): " << ec.message() << std::endl;
                }
            });
    }

    std::cout << "NOTIFY_READY packet sent to all authenticated players "
              << "(player_id=" << static_cast<int>(player_id)
              << ", is_ready=" << is_ready << ")" << std::endl;
}

void PacketSender::SendNotifyDisconnect(uint8_t player_id) {
    // Build NOTIFY_DISCONNECT packet
    network::NotifyDisconnectPacket notify_disconnect;
    notify_disconnect.player_id = network::PlayerId{player_id};
    notify_disconnect.reserved = {0, 0, 0};

    network::PacketBuffer buffer;
    notify_disconnect.Serialize(buffer);
    const auto &data = buffer.Data();

    // Send to all authenticated players via TCP (except the disconnected one)
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        if (client_ref.player_id_ == player_id)
            continue;  // Don't notify the disconnected player themselves

        auto data_copy = std::make_shared<std::vector<uint8_t>>(data);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, player_id](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_DISCONNECT for player "
                              << static_cast<int>(player_id) << ": "
                              << ec.message() << std::endl;
                }
            });
    }

    std::cout << "NOTIFY_DISCONNECT packet sent to all authenticated players "
              << "(player_id=" << static_cast<int>(player_id) << ")"
              << std::endl;
}

void PacketSender::SendNotifyGameSpeed(float speed) {
    // Build NOTIFY_GAME_SPEED packet: header (12 bytes) + float (4 bytes)
    constexpr size_t kHeaderSize = 12;
    constexpr size_t kPayloadSize = 4;
    auto pkt =
        std::make_shared<std::vector<uint8_t>>(kHeaderSize + kPayloadSize);

    // Write header
    (*pkt)[0] = static_cast<uint8_t>(network::PacketType::NotifyGameSpeed);
    (*pkt)[1] = kPayloadSize & 0xFF;         // PayloadSize low byte
    (*pkt)[2] = (kPayloadSize >> 8) & 0xFF;  // PayloadSize high byte
    // TickId (4 bytes) - set to 0 for TCP control messages
    (*pkt)[3] = 0;
    (*pkt)[4] = 0;
    (*pkt)[5] = 0;
    (*pkt)[6] = 0;
    // Reserved (5 bytes)
    (*pkt)[7] = 0;
    (*pkt)[8] = 0;
    (*pkt)[9] = 0;
    (*pkt)[10] = 0;
    (*pkt)[11] = 0;

    // Write float as bytes (little-endian)
    uint32_t speed_bits;
    std::memcpy(&speed_bits, &speed, sizeof(float));
    (*pkt)[kHeaderSize + 0] = (speed_bits >> 0) & 0xFF;
    (*pkt)[kHeaderSize + 1] = (speed_bits >> 8) & 0xFF;
    (*pkt)[kHeaderSize + 2] = (speed_bits >> 16) & 0xFF;
    (*pkt)[kHeaderSize + 3] = (speed_bits >> 24) & 0xFF;

    // Send to all authenticated players via TCP
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        auto data_copy = std::make_shared<std::vector<uint8_t>>(*pkt);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, speed](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_GAME_SPEED (" << speed
                              << "x): " << ec.message() << std::endl;
                }
            });
    }

    std::cout << "NOTIFY_GAME_SPEED packet sent to all authenticated players "
              << "(speed=" << speed << "x)" << std::endl;
}

void PacketSender::SendNotifyDifficulty(uint8_t difficulty) {
    // Build packet: header (12 bytes) + uint8 (1 byte)
    constexpr size_t kHeaderSize = 12;
    constexpr size_t kPayloadSize = 1;
    auto pkt =
        std::make_shared<std::vector<uint8_t>>(kHeaderSize + kPayloadSize);

    // Write header (opcode 0x0C, payload size 1)
    (*pkt)[0] = 0x52;  // Magic byte 'R'
    (*pkt)[1] = 0x54;  // Magic byte 'T'
    (*pkt)[2] = 0;     // Protocol version
    (*pkt)[3] = 0;
    (*pkt)[4] = 0;
    (*pkt)[5] = 0;
    (*pkt)[6] = 0x0C;  // OpCode: SET_DIFFICULTY (reused for notification)
    (*pkt)[7] = 0;     // Reserved
    (*pkt)[8] = kPayloadSize & 0xFF;  // Payload length low
    (*pkt)[9] = (kPayloadSize >> 8) & 0xFF;
    (*pkt)[10] = (kPayloadSize >> 16) & 0xFF;
    (*pkt)[11] = (kPayloadSize >> 24) & 0xFF;

    // Write payload: difficulty level
    (*pkt)[kHeaderSize] = difficulty;

    // Send to all authenticated players via TCP
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;

        auto data_copy = std::make_shared<std::vector<uint8_t>>(*pkt);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, difficulty](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_DIFFICULTY ("
                              << static_cast<int>(difficulty)
                              << "): " << ec.message() << std::endl;
                }
            });
    }

    const char *names[] = {"Easy", "Normal", "Hard"};
    std::cout << "NOTIFY_DIFFICULTY packet sent (level=" << names[difficulty]
              << ")" << std::endl;
}

void PacketSender::SendNotifyKillableProjectiles(bool enabled) {
    // Build packet: header (12 bytes) + uint8 (1 byte)
    constexpr size_t kHeaderSize = 12;
    constexpr size_t kPayloadSize = 1;
    auto pkt =
        std::make_shared<std::vector<uint8_t>>(kHeaderSize + kPayloadSize);

    // Write header (opcode 0x0D, payload size 1)
    (*pkt)[0] = 0x52;  // Magic byte 'R'
    (*pkt)[1] = 0x54;  // Magic byte 'T'
    (*pkt)[2] = 0;     // Protocol version
    (*pkt)[3] = 0;
    (*pkt)[4] = 0;
    (*pkt)[5] = 0;
    (*pkt)[6] = 0x0D;  // OpCode: SET_KILLABLE_PROJECTILES (reused for notification)
    (*pkt)[7] = 0;     // Reserved
    (*pkt)[8] = kPayloadSize & 0xFF;
    (*pkt)[9] = (kPayloadSize >> 8) & 0xFF;
    (*pkt)[10] = (kPayloadSize >> 16) & 0xFF;
    (*pkt)[11] = (kPayloadSize >> 24) & 0xFF;

    // Write payload: enabled flag
    (*pkt)[kHeaderSize] = enabled ? 1 : 0;

    // Send to all authenticated players via TCP
    auto &clients = connection_manager_.GetClients();
    for (auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;

        auto data_copy = std::make_shared<std::vector<uint8_t>>(*pkt);
        client_ref.tcp_socket_.async_send(boost::asio::buffer(*data_copy),
            [data_copy, enabled](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Failed to send NOTIFY_KILLABLE_PROJECTILES ("
                              << (enabled ? "ON" : "OFF")
                              << "): " << ec.message() << std::endl;
                }
            });
    }

    std::cout << "NOTIFY_KILLABLE_PROJECTILES packet sent (enabled="
              << (enabled ? "ON" : "OFF") << ")" << std::endl;
}

void SendSnapshotPlayerState(int tick, network::EntityState entity_state,
    network::PacketBuffer &buffer) {
    // Serialize entity state into the payload
    // Write WORLD_SNAPSHOT header (opcode 0x20 with entity data as payload)
    network::CommonHeader header(
        static_cast<uint8_t>(network::PacketType::WorldSnapshot),
        20,    // payload_size = 20 bytes (one EntityState with health)
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
        21,  // payload_size = 21 bytes (one EntityState with animation/health)
        tick,  // tick_id
        0,     // packet_index = 0
        1,     // packet_count = 1
        1);    // entity_type = 1 (Enemy)
    buffer.WriteHeader(header);

    // Serialize entity state into the payload
    entity_state.Serialize(buffer, network::EntityState::EntityType::Enemy);
}

void SendSnapshotObstacleState(int tick, network::EntityState entity_state,
    network::PacketBuffer &buffer) {
    // Obstacles use the same serialization format as enemies but with
    // different entity_type in header (3 = Obstacle)
    network::CommonHeader header(
        static_cast<uint8_t>(network::PacketType::WorldSnapshot),
        21,    // payload_size = 21 bytes (same as enemy format)
        tick,  // tick_id
        0,     // packet_index = 0
        1,     // packet_count = 1
        3);    // entity_type = 3 (Obstacle)
    buffer.WriteHeader(header);

    // Serialize entity state using enemy format (position, velocity, health)
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
    } else if (entity_state.entity_type == 3) {  // OBSTACLE
        SendSnapshotObstacleState(tick, entity_state, buffer);
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

}  // namespace server
