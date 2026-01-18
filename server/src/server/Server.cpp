#include "server/Server.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/systems/Systems.hpp"
#include "server/systems/WorldGenSystem.hpp"

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

    // Initialize WorldGen
    worldgen_loader_ = std::make_unique<worldgen::WorldGenConfigLoader>();
    worldgen_loader_->SetLogCallback(
        [](worldgen::LogLevel level, const std::string &msg) {
            std::string level_str;
            switch (level) {
                case worldgen::LogLevel::kInfo:
                    level_str = "[WORLDGEN INFO]";
                    break;
                case worldgen::LogLevel::kWarning:
                    level_str = "[WORLDGEN WARN]";
                    break;
                case worldgen::LogLevel::kError:
                    level_str = "[WORLDGEN ERROR]";
                    break;
            }
            std::cout << level_str << " " << msg << std::endl;
        });

    // Load WGF files
    if (!worldgen_loader_->LoadFromDirectories(
            "assets/worldgen/core", "assets/worldgen/user")) {
        std::cerr << "[WARNING] Failed to load WorldGen files, procedural "
                     "generation disabled"
                  << std::endl;
    } else {
        worldgen_manager_ =
            std::make_unique<worldgen::WorldGenManager>(*worldgen_loader_);
        std::cout << "[WorldGen] Loaded "
                  << worldgen_loader_->GetAllWGFs().size() << " WGF files"
                  << std::endl;
    }

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

void Server::PromptLevelSelection() {
    if (!worldgen_manager_) {
        std::cout << "[WARNING] WorldGen not available, using fallback "
                     "spawning.\n";
        return;
    }

    // Load levels from the levels directory
    std::string levels_path = "assets/worldgen/levels";
    int level_count = 0;
    try {
        for (const auto &entry :
            std::filesystem::directory_iterator(levels_path)) {
            if (entry.is_regular_file()) {
                const auto &path = entry.path();
                if (path.string().ends_with(".level.json")) {
                    if (worldgen_manager_->LoadLevelFromFile(path.string())) {
                        level_count++;
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "[WARNING] Could not scan levels directory: " << e.what()
                  << std::endl;
    }

    const auto &levels = worldgen_manager_->GetAllLevels();

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        R-TYPE J.A.M.E.S. SERVER\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    std::cout << "Available Game Modes:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  0. Endless Mode (Infinite)\n";

    for (size_t i = 0; i < levels.size(); ++i) {
        const auto &level = levels[i];
        std::string mode_type = level.is_endless ? "Infinite" : "Finite";
        std::cout << "  " << (i + 1) << ". " << level.name << " (" << mode_type
                  << ")\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "\n";
    std::cout << "Enter your choice [0-" << levels.size() << "]: ";
    std::cout.flush();

    int choice = -1;
    while (true) {
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number [0-"
                      << levels.size() << "]: ";
            std::cout.flush();
            continue;
        }

        if (choice < 0 || choice > static_cast<int>(levels.size())) {
            std::cout << "Invalid choice. Please enter a number [0-"
                      << levels.size() << "]: ";
            std::cout.flush();
            continue;
        }

        break;
    }

    if (choice == 0) {
        selected_level_uuid_.clear();
        std::cout << "\n[Server] Selected: Endless Mode\n";
    } else {
        const auto &selected = levels[choice - 1];
        selected_level_uuid_ = selected.uuid;
        std::string mode_type = selected.is_endless ? "Infinite" : "Finite";
        std::cout << "\n[Server] Selected: " << selected.name << " ("
                  << mode_type << ")\n";
    }

    std::cout << "\nWaiting for players to connect...\n\n";
}

void Server::SetSelectedLevel(const std::string &level_uuid) {
    selected_level_uuid_ = level_uuid;
}

std::vector<std::pair<std::string, bool>> Server::GetAvailableLevels() const {
    std::vector<std::pair<std::string, bool>> result;
    if (worldgen_manager_) {
        for (const auto &level : worldgen_manager_->GetAllLevels()) {
            result.emplace_back(level.name, level.is_endless);
        }
    }
    return result;
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

void Server::NotifyPlayerDeath(uint8_t player_id, int final_score) {
    alive_players_--;

    // Track death order and final score
    death_order_counter_++;
    if (player_records_.find(player_id) != player_records_.end()) {
        player_records_[player_id].death_order = death_order_counter_;
        player_records_[player_id].is_alive = false;
        player_records_[player_id].score = final_score;
        std::cout << "[Server::NotifyPlayerDeath] Player "
                  << static_cast<int>(player_id)
                  << " died (order: " << death_order_counter_
                  << ", score: " << final_score << ")" << std::endl;
    }

    std::cout << "[Server::NotifyPlayerDeath] Alive players: "
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

    // Get score before destroying entity
    int final_score = 0;
    auto &player_tags = registry_.GetComponents<Component::PlayerTag>();
    for (std::size_t i = 0; i < player_tags.size(); ++i) {
        if (!player_tags.has(i))
            continue;
        if (player_tags[i].value().playerNumber ==
            static_cast<int>(player_id)) {
            final_score = player_tags[i].value().score;
            break;
        }
    }

    // Destroy the player's entity and update tracking only if entity was found
    if (DestroyPlayerEntity(player_id) && alive_players_ > 0) {
        NotifyPlayerDeath(player_id, final_score);
    }

    std::cout << "[Server::HandlePlayerDisconnect] Player "
              << static_cast<int>(player_id)
              << " left. Alive: " << alive_players_ << "/" << total_players_
              << std::endl;

    // Check if all players have left/died
    if (connection_manager_.GetAuthenticatedCount() == 0 ||
        alive_players_ <= 0) {
        std::cout << "[Server] All players gone! Game Over." << std::endl;

        // Build leaderboard and send GAME_END
        auto leaderboard = BuildLeaderboard(false);  // false = game over
        uint8_t game_mode =
            (worldgen_manager_ && !worldgen_manager_->IsEndlessMode()) ? 0 : 1;
        packet_sender_.SendGameEnd(0, game_mode, leaderboard);
        ResetToLobby();
    }
}

void Server::ResetToLobby() {
    std::cout << "Resetting server to lobby state..." << std::endl;

    // Stop the game loop
    Stop();

    // Clear all entities from the registry
    registry_.ClearAllEntities();

    // Reset WorldGen manager state BEFORE destroying the system
    // This prevents the manager from using stale callbacks during reset
    if (worldgen_manager_) {
        worldgen_manager_->Stop();           // Stop activity first
        worldgen_manager_->ClearCallback();  // Clear the spawn callback
    }

    // Now safe to destroy the system
    worldgen_system_.reset();

    // Reset player tracking
    player_records_.clear();
    death_order_counter_ = 0;

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

    // Update WorldGen system if initialized
    // scroll_speed should match enemy/background movement (~200 pixels/second)
    if (worldgen_system_) {
        worldgen_system_->Update(1.0f / 60.0f, 200.0f, registry_);
    }

    // Update alive player scores in records (for accurate leaderboard)
    auto &player_tags = registry_.GetComponents<Component::PlayerTag>();
    for (std::size_t i = 0; i < player_tags.size(); ++i) {
        if (!player_tags.has(i))
            continue;
        uint8_t pid =
            static_cast<uint8_t>(player_tags[i].value().playerNumber);
        if (player_records_.find(pid) != player_records_.end() &&
            player_records_[pid].is_alive) {
            player_records_[pid].score = player_tags[i].value().score;
        }
    }

    // Determine game mode
    bool is_finite = worldgen_manager_ &&
                     worldgen_manager_->IsLevelComplete() &&
                     !worldgen_manager_->IsEndlessMode();
    bool is_endless = !worldgen_manager_ || worldgen_manager_->IsEndlessMode();
    uint8_t game_mode = is_endless ? 1 : 0;

    // Check for level completion (victory condition for finite levels)
    if (worldgen_manager_ && worldgen_manager_->IsLevelComplete() &&
        !worldgen_manager_->IsEndlessMode()) {
        if (!victory_pending_) {
            std::cout << "[Server] Level complete! Starting victory delay..."
                      << std::endl;
            victory_pending_ = true;
            victory_timer_ = 0.0f;
        } else {
            // Accumulate time
            victory_timer_ += g_frame_delta_ms / 1000.0f;

            if (victory_timer_ >= VICTORY_DELAY_SEC) {
                std::cout
                    << "[Server] Victory delay complete. Determining winner..."
                    << std::endl;

                // For finite mode:
                // - If all players alive, winner is the one with highest score
                // - If some died, winner is the one who survived longest
                uint8_t winner_id =
                    DetermineWinner(true);  // true = victory scenario

                // Build leaderboard with proper winner flags
                auto leaderboard = BuildLeaderboard(true, winner_id);

                std::cout << "[Server] Sending GAME_END (winner="
                          << static_cast<int>(winner_id) << ")" << std::endl;

                packet_sender_.SendGameEnd(winner_id, game_mode, leaderboard);

                // Reset server to lobby state
                ResetToLobby();
                victory_pending_ = false;
                victory_timer_ = 0.0f;
                return;
            }
        }
    }

    // Check for game over condition (all players dead)
    if (AreAllPlayersDead()) {
        if (!game_over_pending_) {
            std::cout << "[Server] All players are dead! Starting game over "
                         "delay..."
                      << std::endl;
            game_over_pending_ = true;
            game_over_timer_ = 0.0f;
        } else {
            // Accumulate time (assuming 16ms per tick)
            game_over_timer_ += g_frame_delta_ms / 1000.0f;

            if (game_over_timer_ >= GAME_OVER_DELAY_SEC) {
                std::cout << "[Server] Game over delay complete. Determining "
                             "winner..."
                          << std::endl;

                // For game over (all dead):
                // - In infinite mode: winner is who survived longest (died
                // last)
                // - If tie (died same frame), winner is who had more points
                uint8_t winner_id =
                    DetermineWinner(false);  // false = all dead scenario

                // Build leaderboard
                auto leaderboard = BuildLeaderboard(false, winner_id);

                std::cout << "[Server] Sending GAME_END (winner="
                          << static_cast<int>(winner_id) << ")" << std::endl;

                packet_sender_.SendGameEnd(winner_id, game_mode, leaderboard);

                // Reset server to lobby state
                ResetToLobby();
                game_over_pending_ = false;
                game_over_timer_ = 0.0f;
                return;
            }
        }
    } else {
        // Reset game over pending if players are alive again (shouldn't
        // happen but safe)
        game_over_pending_ = false;
        game_over_timer_ = 0.0f;
    }

    SendSnapshotsToAllClients();
}

uint8_t Server::DetermineWinner(bool is_victory) {
    std::vector<std::pair<uint8_t, const PlayerDeathRecord *>> candidates;

    // Collect all player records
    for (const auto &[pid, record] : player_records_) {
        candidates.emplace_back(pid, &record);
    }

    if (candidates.empty()) {
        return 0;  // No players
    }

    // For finite mode with victory (level complete):
    // - If all alive: highest score wins
    // - If some died: whoever survived longest (lowest death_order among dead,
    //   or alive) wins, tie-break by score
    if (is_victory) {
        // Check if anyone is still alive
        bool anyone_alive = false;
        for (const auto &[pid, rec] : candidates) {
            if (rec->is_alive) {
                anyone_alive = true;
                break;
            }
        }

        if (anyone_alive) {
            // All alive survivors: pick highest score among alive
            uint8_t winner = 0;
            int best_score = -999999;
            for (const auto &[pid, rec] : candidates) {
                if (rec->is_alive && rec->score > best_score) {
                    best_score = rec->score;
                    winner = pid;
                }
            }
            return winner;
        }
    }

    // For infinite mode or when all dead:
    // Winner is whoever survived longest (died last = highest death_order)
    // Tie-break: highest score
    uint8_t winner = 0;
    int best_death_order = -1;  // Higher = died later = better
    int best_score = -999999;

    for (const auto &[pid, rec] : candidates) {
        int death_order = rec->is_alive ? 99999 : rec->death_order;

        if (death_order > best_death_order ||
            (death_order == best_death_order && rec->score > best_score)) {
            best_death_order = death_order;
            best_score = rec->score;
            winner = pid;
        }
    }

    return winner;
}

std::vector<network::PlayerScoreData> Server::BuildLeaderboard(
    bool is_victory, uint8_t winner_id) {
    std::vector<network::PlayerScoreData> leaderboard;

    for (const auto &[pid, record] : player_records_) {
        network::PlayerScoreData entry;
        entry.player_id = network::PlayerId{pid};
        entry.SetName(record.username);
        entry.score = static_cast<uint32_t>(std::max(0, record.score));
        entry.death_order = static_cast<uint8_t>(record.death_order);
        entry.is_winner = (pid == winner_id) ? 1 : 0;
        leaderboard.push_back(entry);
    }

    // Sort by: 1) is_winner desc, 2) death_order asc (0=alive first), 3) score
    // desc
    std::sort(leaderboard.begin(), leaderboard.end(),
        [](const network::PlayerScoreData &a,
            const network::PlayerScoreData &b) {
            if (a.is_winner != b.is_winner)
                return a.is_winner > b.is_winner;
            // death_order 0 = alive, higher = died later
            // We want alive first, then died last (highest death_order)
            if (a.death_order == 0 && b.death_order != 0)
                return true;
            if (b.death_order == 0 && a.death_order != 0)
                return false;
            if (a.death_order != b.death_order)
                return a.death_order > b.death_order;  // Died later = better
            return a.score > b.score;
        });

    return leaderboard;
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
