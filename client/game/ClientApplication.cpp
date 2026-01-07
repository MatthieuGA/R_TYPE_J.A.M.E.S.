#include "game/ClientApplication.hpp"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "adapters/SFMLInputAdapters.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "game/InitRegistry.hpp"
#include "game/SnapshotTracker.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "input/Event.hpp"
#include "network/Network.hpp"

namespace Rtype::Client {
// Parse Player

void UpdateNetworkIdTick(GameWorld &game_world, size_t entity_index,
    const ClientApplication::ParsedEntity &entity_data, uint32_t tick) {
    if (game_world.registry_.GetComponents<Component::NetworkId>().has(
            entity_index)) {
        auto &net_id =
            game_world.registry_
                .GetComponents<Component::NetworkId>()[entity_index];
        if (net_id.has_value()) {
            net_id->last_processed_tick = tick;
        }
    }
}

/**
 * @brief Apply snapshot data to the ECS registry.
 *
 * Updates entity positions and rotations based on server snapshot.
 * Creates or updates entities with NetworkId components.
 *
 * @param game_world The game world containing the registry
 * @param snapshot The snapshot packet received from server
 */
void ClientApplication::ApplySnapshotToRegistry(
    GameWorld &game_world, const client::SnapshotPacket &snapshot) {
    auto entities = ParseSnapshotData(snapshot);

    auto &net_ids = game_world.registry_.GetComponents<Component::NetworkId>();

    SnapshotTracker::GetInstance().UpdateLastProcessedTick(snapshot.tick);

    for (const auto &entity_data : entities) {
        // Find entity with matching NetworkId
        std::optional<size_t> entity_index;

        // Search for existing entity with this NetworkId
        for (auto &&[idx, net_id] : make_indexed_zipper(net_ids)) {
            if (net_id.id == static_cast<int>(entity_data.entity_id)) {
                entity_index = idx;
                break;
            }
        }

        // If entity doesn't exist, create it
        if (!entity_index.has_value()) {
            CreateNewEntity(
                game_world, snapshot.tick, entity_data, entity_index);
        } else {
            UpdateExistingEntity(
                game_world, entity_index.value(), entity_data);
            UpdateNetworkIdTick(
                game_world, entity_index.value(), entity_data, snapshot.tick);
        }
    }
}

bool ClientApplication::ConnectToServerWithRetry(
    GameWorld &game_world, const ClientConfig &config) {
    bool connected = false;

    for (int retry = 0; retry < kMaxRetries && !connected; ++retry) {
        if (retry > 0) {
            std::cout << "[Network] Retry " << retry << "/" << kMaxRetries
                      << "..." << std::endl;
        } else {
            std::cout << "[Network] Attempting to connect to server at "
                      << config.server_ip << ":" << config.tcp_port << "..."
                      << std::endl;
        }

        // Reset rejection status before each attempt
        game_world.server_connection_->ResetRejectionStatus();

        game_world.server_connection_->ConnectToServer(config.username);

        // Poll io_context to process connection attempt
        for (int i = 0; i < kPollIterations &&
                        !game_world.server_connection_->is_connected();
            ++i) {
            game_world.io_context_.poll();
            sf::sleep(sf::milliseconds(kPollDelayMs));

            // Check if we got a permanent rejection (e.g., game in progress)
            if (game_world.server_connection_->WasRejectedPermanently()) {
                std::cerr << "[Network] Connection rejected: Game in progress."
                          << std::endl;
                std::cerr << "[Network] Cannot join - please wait for the "
                          << "current game to finish." << std::endl;
                return false;  // Don't retry
            }
        }

        connected = game_world.server_connection_->is_connected();

        if (!connected && retry < kMaxRetries - 1) {
            // Check again for permanent rejection before retrying
            if (game_world.server_connection_->WasRejectedPermanently()) {
                std::cerr << "[Network] Connection rejected: Game in progress."
                          << std::endl;
                return false;  // Don't retry
            }

            std::cout << "[Network] Connection attempt " << (retry + 1)
                      << " failed. Retrying..." << std::endl;
            // Reset io_context for next attempt
            game_world.io_context_.restart();
            sf::sleep(sf::milliseconds(kRetryDelayMs));
        }
    }

    if (!connected) {
        std::cerr << "[Network] Failed to connect to server after "
                  << kMaxRetries << " attempts." << std::endl;
        std::cerr << "[Network] Please check that the server is running at "
                  << config.server_ip << ":" << config.tcp_port << std::endl;
        return false;
    }

    std::cout << "[Network] Successfully connected to server!" << std::endl;
    return true;
}

void ClientApplication::RunGameLoop(GameWorld &game_world) {
    while (game_world.window_.isOpen()) {
        // Handle window events
        sf::Event sfml_event;
        while (game_world.window_.pollEvent(sfml_event)) {
            Engine::Input::Event event;
            if (Adapters::FromSFMLEvent(sfml_event, event)) {
                if (event.type == Engine::Input::EventType::Closed) {
                    game_world.window_.close();
                }
            }
        }

        // Poll asio for network events
        game_world.io_context_.poll();

        // Check if game has started (received GAME_START from server)
        if (game_world.server_connection_) {
            if (game_world.server_connection_->HasGameStarted()) {
                std::cout
                    << "[Client] Game started! Switching to GameLevel scene"
                    << std::endl;
                std::cout.flush();
                // Reset the flag to avoid re-triggering
                game_world.server_connection_->ResetGameStarted();

                // Switch scene to GameLevel
                auto &scene_comps =
                    game_world.registry_
                        .GetComponents<Component::SceneManagement>();
                std::cout << "[Client] SceneManagement components count: "
                          << scene_comps.size() << std::endl;
                std::cout.flush();

                bool found = false;
                for (auto &&[i, gs] : make_indexed_zipper(scene_comps)) {
                    found = true;
                    std::cout << "[Client] Found SceneManagement at index "
                              << i << ", current=" << gs.current
                              << ", next=" << gs.next << std::endl;
                    std::cout.flush();
                    gs.next = "GameLevel";
                    std::cout << "[Client] Set next to GameLevel" << std::endl;
                    std::cout.flush();
                    break;  // Only need to set once
                }

                if (!found) {
                    std::cerr << "[Client] ERROR: No SceneManagement "
                                 "component found!"
                              << std::endl;
                    std::cerr.flush();
                }
            }
        }

        // Poll and apply UDP snapshots
        // (outside game-start check, runs each frame)
        if (game_world.server_connection_) {
            // Process all queued snapshots each frame to avoid backlog
            // static int packet_count = 0;
            while (true) {
                auto snapshot = game_world.server_connection_->PollSnapshot();
                if (!snapshot.has_value())
                    break;

                ApplySnapshotToRegistry(game_world, *snapshot);
            }
        }

        // Calculate delta time at the beginning of the frame
        game_world.last_delta_ =
            game_world.delta_time_clock_.Restart().AsSeconds();

        // Clear, update, and render
        game_world.window_.clear(sf::Color::Black);
        game_world.registry_.RunSystems();
        game_world.window_.display();
    }
}

void ClientApplication::InitializeApplication(GameWorld &game_world) {
    InitRegistry(game_world, *game_world.audio_manager_);
    InitSceneLevel(game_world.registry_);
}

}  // namespace Rtype::Client
