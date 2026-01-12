#include "server/Server.hpp"

#include <iostream>
#include <utility>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/systems/Systems.hpp"

namespace server {

// Static instance pointer for system callbacks
Server *Server::instance_ = nullptr;

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(config, io_context),
      registry_(),
      tick_timer_(io_context),
      running_(false),
      connection_manager_(config.GetMaxPlayers(), config.GetUdpPort()),
      packet_sender_(connection_manager_, network_),
      packet_handler_(connection_manager_, packet_sender_, network_) {
    // Set singleton instance
    instance_ = this;
    // Set game start callback
    packet_handler_.SetGameStartCallback([this]() { Start(); });
    last_tick_time_ = std::chrono::steady_clock::now();
    // Set callback to check if game is running
    packet_handler_.SetIsGameRunningCallback([this]() { return running_; });
}

Server::~Server() {
    running_ = false;
}

void Server::Initialize() {
    std::cout << "Initializing server..." << std::endl;
    RegisterComponents();
    RegisterSystems();

    // Register packet handlers
    packet_handler_.RegisterHandlers();

    // Register TCP accept callback
    network_.GetTcp().SetAcceptCallback(
        [this](boost::asio::ip::tcp::socket socket) {
            HandleTcpAccept(std::move(socket));
        });

    // Register UDP receive callback to update client endpoints
    // Prefer using player_id carried in PLAYER_INPUT (discovery) packets
    network_.SetUdpReceiveCallback(
        [this](const boost::asio::ip::udp::endpoint &endpoint,
            const std::vector<uint8_t> &data) {
            // Basic validation: need at least header + 1 payload byte
            if (data.size() >= 13) {
                uint8_t opcode = data[0];
                // PLAYER_INPUT opcode is 0x10
                if (opcode == 0x10) {
                    bool handled = HandleUdpPlayerInput(endpoint, data);
                    if (handled) {
                        return;  // packet processed
                    }
                }
            }

            // Fallback: match by IP address (legacy behavior)
            ClientConnection *client =
                connection_manager_.FindClientByIp(endpoint.address());
            if (client) {
                connection_manager_.UpdateClientUdpEndpoint(
                    client->client_id_, endpoint);
            }
        });

    std::cout << "Server initialized successfully" << std::endl;
}

void Server::Start() {
    if (running_) {
        return;
    }
    std::cout << "Starting game..." << std::endl;
    running_ = true;

    // Clear any leftover entities from previous games before setting up new
    // one
    registry_.ClearAllEntities();

    SetupEntitiesGame();
    SetupGameTick();
}

void Server::Stop() {
    std::cout << "Stopping game..." << std::endl;
    running_ = false;
    tick_timer_.cancel();
    std::cout << "Game stopped" << std::endl;
}

bool Server::AreAllPlayersDead() {
    // Simple check: if we had players and none are alive, game over
    if (total_players_ > 0 && alive_players_ <= 0) {
        std::cout << "[Server::AreAllPlayersDead] All players dead! "
                  << "total=" << total_players_ << ", alive=" << alive_players_
                  << std::endl;
        return true;
    }
    return false;
}

void Server::NotifyPlayerDeath() {
    alive_players_--;
    std::cout << "[Server::NotifyPlayerDeath] Player died. Alive players: "
              << alive_players_ << "/" << total_players_ << std::endl;
}

bool Server::DestroyPlayerEntity(uint8_t player_id) {
    auto &player_tags = registry_.GetComponents<Component::PlayerTag>();

    for (std::size_t i = 0; i < player_tags.size(); ++i) {
        if (!player_tags.has(i))
            continue;

        if (player_tags[i].value().playerNumber ==
            static_cast<int>(player_id)) {
            auto entity = registry_.EntityFromIndex(i);
            registry_.KillEntity(entity);
            std::cout << "[Server::DestroyPlayerEntity] Destroyed entity for "
                      << "player_id=" << static_cast<int>(player_id)
                      << std::endl;
            return true;
        }
    }
    std::cout << "[Server::DestroyPlayerEntity] No entity found for player_id="
              << static_cast<int>(player_id) << std::endl;
    return false;
}

void Server::HandlePlayerDisconnect(uint8_t player_id) {
    if (!running_) {
        return;  // Not in game, nothing to do
    }

    // Destroy the player's entity and update tracking only if entity was found
    if (DestroyPlayerEntity(player_id) && alive_players_ > 0) {
        alive_players_--;
    }

    std::cout << "[Server::HandlePlayerDisconnect] Player "
              << static_cast<int>(player_id)
              << " left. Alive: " << alive_players_ << "/" << total_players_
              << std::endl;

    // Check if all players have left/died
    if (connection_manager_.GetAuthenticatedCount() == 0 ||
        alive_players_ <= 0) {
        std::cout << "[Server] All players gone! Game Over." << std::endl;
        packet_sender_.SendGameEnd(0);
        ResetToLobby();
    }
}

void Server::ResetToLobby() {
    std::cout << "Resetting server to lobby state..." << std::endl;

    // Stop the game loop
    Stop();

    // Clear all entities from the registry
    registry_.ClearAllEntities();

    // Reset all client ready states
    connection_manager_.ResetAllReadyStates();

    std::cout << "Server reset to lobby. Waiting for players to ready up..."
              << std::endl;
}

void Server::Close() {
    std::cout << "Closing server..." << std::endl;
    Stop();
    // Client sockets will be closed automatically when connection_manager_ is
    // destroyed (when the Server object is destroyed)
    std::cout << "Server closed" << std::endl;
}

void Server::SetupGameTick() {
    if (!running_) {
        return;
    }

    tick_timer_.expires_after(std::chrono::milliseconds(kTickTimerMs));
    tick_timer_.async_wait([this](const boost::system::error_code &ec) {
        if (!ec && running_) {
            // Calculate real elapsed time since last tick and update global
            // frame delta
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = now - last_tick_time_;
            UpdateFrameDeltaFromSeconds(elapsed.count());
            last_tick_time_ = now;

            Update();
            SetupGameTick();
        }
    });
}

void Server::Update() {
    registry_.run_systems();

    // Check for game over condition (all players dead)
    if (AreAllPlayersDead()) {
        std::cout << "[Server] All players are dead! Game Over." << std::endl;

        // Send GAME_END packet to all clients (0 = no winner, game lost)
        packet_sender_.SendGameEnd(0);

        // Reset server to lobby state
        ResetToLobby();
        return;
    }

    SendSnapshotsToAllClients();
}

Engine::registry &Server::GetRegistry() {
    return registry_;
}

uint32_t Server::GetNextNetworkId() {
    static uint32_t next_id = 1;
    uint32_t id = next_id++;
    return id;
}

void Server::HandleTcpAccept(boost::asio::ip::tcp::socket socket) {
    // Add client to connection manager
    uint32_t client_id = connection_manager_.AddClient(std::move(socket));

    // Start handling messages immediately
    packet_handler_.StartReceiving(client_id);
}

}  // namespace server
