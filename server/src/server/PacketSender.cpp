#include "server/PacketSender.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "server/PacketBuffer.hpp"

namespace server {

PacketSender::PacketSender(ClientConnectionManager &connection_manager)
    : connection_manager_(connection_manager) {}

void PacketSender::SendConnectAck(ClientConnection &client,
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

void PacketSender::SendGameStart() {
    // Send to all authenticated players
    // Note: We need non-const access to use tcp_socket_.async_send()

    // Serialize packet once before the loop (same for all clients)
    network::GameStartPacket game_start;
    // TODO(Matthieu-GA): Assign actual controlled entity ID from ECS
    game_start.controlled_entity_id = network::EntityId{0};

    network::PacketBuffer buffer;
    game_start.Serialize(buffer);
    const auto &data = buffer.Data();

    const auto &clients = connection_manager_.GetClients();
    for (const auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0) {
            continue;  // Skip unauthenticated clients
        }
        // Get non-const reference to the client for socket operations
        ClientConnection &client = connection_manager_.GetClient(client_id);

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
