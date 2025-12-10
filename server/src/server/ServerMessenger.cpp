#include "server/ServerMessenger.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "server/PacketDispatcher.hpp"

namespace server {

ServerMessenger::ServerMessenger(ClientConnectionManager &connection_manager)
    : connection_manager_(connection_manager), dispatcher_(nullptr) {}

void ServerMessenger::SetDispatcher(PacketDispatcher *dispatcher) {
    dispatcher_ = dispatcher;
}

void ServerMessenger::StartReceiving(uint32_t client_id) {
    HandleClientMessages(client_id);
}

void ServerMessenger::HandleClientMessages(uint32_t client_id) {
    if (!connection_manager_.HasClient(client_id))
        return;  // Client already disconnected

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
                if (!connection_manager_.HasClient(client_id))
                    return;  // Client was removed

                ClientConnection &client =
                    connection_manager_.GetClient(client_id);
                client.last_activity_ = std::chrono::steady_clock::now();

                // Parse incoming TCP packet using PacketFactory
                network::PacketParseResult result =
                    network::DeserializePacket(buffer->data(), bytes_read);

                // Dispatch to packet dispatcher if available
                if (dispatcher_) {
                    dispatcher_->Dispatch(client_id, result);
                } else {
                    std::cerr << "No dispatcher set for ServerMessenger"
                              << std::endl;
                }

                // Continue handling messages if connection still exists
                if (connection_manager_.HasClient(client_id))
                    HandleClientMessages(client_id);
            } else {
                // EOF without error - client disconnected gracefully
                std::cout << "Client " << client_id
                          << " disconnected gracefully" << std::endl;
                connection_manager_.RemoveClient(client_id);
            }
        });
}

void ServerMessenger::SendConnectAck(ClientConnection &client,
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

void ServerMessenger::SendGameStart() {
    // Send to all authenticated players
    // Note: We need non-const access to use tcp_socket_.async_send()
    const auto &clients = connection_manager_.GetClients();
    for (const auto &[client_id, client_ref] : clients) {
        if (client_ref.player_id_ == 0)
            continue;  // Skip unauthenticated clients

        // Get non-const reference to the client for socket operations
        ClientConnection &client = connection_manager_.GetClient(client_id);

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
