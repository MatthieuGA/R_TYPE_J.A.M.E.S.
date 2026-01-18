#include "game/ClientApplication.hpp"

#include <cstdio>
#include <iostream>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "game/InitRegistry.hpp"
#include "game/SnapshotTracker.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "network/Network.hpp"
#include "platform/OSEvent.hpp"
#include "platform/SFMLEventSource.hpp"

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

    SnapshotTracker::GetInstance().UpdateLastProcessedTick(snapshot.tick);

    for (const auto &entity_data : entities) {
        // Re-fetch net_ids each iteration because CreateNewEntity may resize
        // the sparse array
        auto &net_ids =
            game_world.registry_.GetComponents<Component::NetworkId>();

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

namespace {
/**
 * @brief Log game-in-progress rejection message.
 *
 * Helper function to avoid duplicating the rejection error message.
 */
void LogGameInProgressRejection() {
    std::cerr << "[Network] Connection rejected: Game in progress."
              << std::endl;
    std::cerr << "[Network] Cannot join - please wait for the "
              << "current game to finish." << std::endl;
}
}  // namespace

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
                LogGameInProgressRejection();
                return false;  // Don't retry
            }
        }

        connected = game_world.server_connection_->is_connected();

        if (!connected && retry < kMaxRetries - 1) {
            // Check again for permanent rejection before retrying
            if (game_world.server_connection_->WasRejectedPermanently()) {
                LogGameInProgressRejection();
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
    while (game_world.window_->IsOpen()) {
        // Handle window events via platform event source
        Engine::Platform::OSEvent os_event;
        while (game_world.event_source_->Poll(os_event)) {
            if (os_event.type == Engine::Platform::OSEventType::Closed) {
                game_world.window_->Close();
            }
        }

        // Poll asio for network events
        game_world.io_context_.poll();

        // Check for unexpected disconnection (server crashed/closed)
        if (game_world.server_connection_ &&
            game_world.server_connection_->WasDisconnectedUnexpectedly()) {
            std::cerr << "[Client] Lost connection to server!" << std::endl;
            std::cerr << "[Client] The server may have shut down."
                      << std::endl;
            game_world.window_->Close();
            break;
        }

        // Check if game has started (received GAME_START from server)
        if (game_world.server_connection_) {
            if (game_world.server_connection_->HasGameStarted()) {
                // Reset the flag to avoid re-triggering
                game_world.server_connection_->ResetGameStarted();

                // Switch scene to GameLevel
                auto &scene_comps =
                    game_world.registry_
                        .GetComponents<Component::SceneManagement>();

                for (auto &&[i, gs] : make_indexed_zipper(scene_comps)) {
                    gs.next = "GameLevel";
                    break;  // Only need to set once
                }
            }
        }

        // Poll and apply UDP snapshots
        // (outside game-start check, runs each frame)
        if (game_world.server_connection_) {
            // Process all queued snapshots each frame to avoid backlog
            while (true) {
                auto snapshot = game_world.server_connection_->PollSnapshot();
                if (!snapshot.has_value())
                    break;

                ApplySnapshotToRegistry(game_world, *snapshot);
            }
        }

        // Calculate delta time at the beginning of the frame
        float raw_delta = game_world.delta_time_clock_.Restart().AsSeconds();
        game_world.last_delta_ = raw_delta * game_world.game_speed_;

        // Ensure the SFML OpenGL context is active on this thread before
        // running systems that may load textures/shaders.
        game_world.GetNativeWindow().setActive(true);

        // Clear, update, and render
        game_world.GetNativeWindow().clear(sf::Color::Black);
        game_world.registry_.RunSystems();
        game_world.window_->Display();
    }
}

void ClientApplication::InitializeApplication(GameWorld &game_world) {
    InitRegistry(game_world, *game_world.audio_manager_);
    InitSceneLevel(game_world.registry_);
}

}  // namespace Rtype::Client
