#include "server/ClientConnectionManager.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace server {

ClientConnectionManager::ClientConnectionManager(
    uint8_t max_clients, uint16_t server_udp_port)
    : next_client_id_(1),
      next_player_id_(1),
      max_clients_(max_clients),
      server_udp_port_(server_udp_port) {}

uint32_t ClientConnectionManager::AddClient(
    boost::asio::ip::tcp::socket socket) {
    uint32_t client_id = AssignClientId();
    std::cout << "New TCP connection accepted, assigned client ID "
              << client_id << std::endl;

    // Create unauthenticated connection (player_id=0 until authenticated)
    ClientConnection connection(
        client_id, 0, std::move(socket), server_udp_port_);
    clients_.emplace(client_id, std::move(connection));

    return client_id;
}

uint8_t ClientConnectionManager::AuthenticateClient(
    uint32_t client_id, const std::string &username) {
    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        std::cerr << "AuthenticateClient: Client " << client_id << " not found"
                  << std::endl;
        return 0;
    }

    // Check if username is taken
    if (IsUsernameTaken(username)) {
        std::cerr << "Username '" << username << "' already taken"
                  << std::endl;
        return 0;
    }

    // Check if server is full
    if (IsFull()) {
        std::cerr << "Server full, cannot authenticate client " << client_id
                  << std::endl;
        return 0;
    }

    // Assign player_id and username
    uint8_t player_id = AssignPlayerId();
    it->second.player_id_ = player_id;
    it->second.username_ = username;

    std::cout << "Client " << client_id << " authenticated as Player "
              << static_cast<int>(player_id) << " ('" << username << "')"
              << std::endl;

    return player_id;
}

void ClientConnectionManager::RemoveClient(uint32_t client_id) {
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

ClientConnection &ClientConnectionManager::GetClient(uint32_t client_id) {
    return clients_.at(client_id);
}

const ClientConnection &ClientConnectionManager::GetClient(
    uint32_t client_id) const {
    return clients_.at(client_id);
}

bool ClientConnectionManager::HasClient(uint32_t client_id) const {
    return clients_.find(client_id) != clients_.end();
}

bool ClientConnectionManager::IsUsernameTaken(
    const std::string &username) const {
    return std::any_of(
        clients_.begin(), clients_.end(), [&username](const auto &pair) {
            return pair.second.player_id_ != 0 &&
                   pair.second.username_ == username;
        });
}

size_t ClientConnectionManager::GetAuthenticatedCount() const {
    return std::count_if(clients_.begin(), clients_.end(),
        [](const auto &pair) { return pair.second.player_id_ != 0; });
}

bool ClientConnectionManager::IsFull() const {
    return GetAuthenticatedCount() >= max_clients_;
}

uint8_t ClientConnectionManager::GetMaxClients() const {
    return max_clients_;
}

bool ClientConnectionManager::AllPlayersReady() const {
    size_t connected_players = 0;
    bool all_ready = true;

    for (const auto &[id, connection] : clients_) {
        if (connection.player_id_ == 0) {
            continue;  // Skip unauthenticated connections
        }
        connected_players++;
        if (!connection.ready_) {
            all_ready = false;
            break;
        }
    }
    return all_ready;
}

const std::unordered_map<uint32_t, ClientConnection> &
ClientConnectionManager::GetClients() const {
    return clients_;
}

uint32_t ClientConnectionManager::AssignClientId() {
    return next_client_id_++;
}

uint8_t ClientConnectionManager::AssignPlayerId() {
    uint8_t id = next_player_id_++;
    if (next_player_id_ == 0) {
        next_player_id_ = 1;
    }

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
        if (!id_taken) {
            return id;
        }

        id = next_player_id_++;
        if (next_player_id_ == 0) {
            next_player_id_ = 1;
        }
        attempts++;
    }

    throw std::runtime_error("All player IDs exhausted");
}

void ClientConnectionManager::UpdateClientUdpEndpoint(
    uint32_t client_id, const boost::asio::ip::udp::endpoint &endpoint) {
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        it->second.udp_endpoint_ = endpoint;
        // std::cout << "Updat UDP endpoint for client " << client_id << " to "
        //           << endpoint.address().to_string() << ":" <<
        //           endpoint.port()
        //           << std::endl;
    }
}

ClientConnection *ClientConnectionManager::FindClientByIp(
    const boost::asio::ip::address &ip_address) {
    for (auto &[client_id, connection] : clients_) {
        try {
            auto tcp_remote_endpoint =
                connection.tcp_socket_.remote_endpoint();
            if (tcp_remote_endpoint.address() == ip_address) {
                return &connection;
            }
        } catch (const std::exception &e) {
            // Socket might be disconnected
            continue;
        }
    }
    return nullptr;
}

ClientConnection *ClientConnectionManager::FindClientByPlayerId(
    uint8_t player_id) {
    for (auto &[client_id, connection] : clients_) {
        if (connection.player_id_ == player_id) {
            return &connection;
        }
    }
    return nullptr;
}

}  // namespace server
