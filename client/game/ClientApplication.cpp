#include "game/ClientApplication.hpp"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "game/InitRegistry.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "network/Network.hpp"

namespace Rtype::Client {

/**
 * @brief Represents a parsed entity from a snapshot.
 *
 * Stores entity state information received from server.
 */
struct ParsedEntity {
    uint32_t entity_id;
    uint8_t entity_type;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t angle;
    uint16_t velocity_x;
    uint16_t velocity_y;
};

/**
 * @brief Parse snapshot data from UDP packet.
 *
 * Deserializes EntityState structures from the snapshot payload.
 * Current implementation: Server sends a single EntityState directly (12
 * bytes) without WorldSnapshotPacket header.
 *
 * @param snapshot The snapshot packet received from server
 * @return Vector of parsed entities
 */
static std::vector<ParsedEntity> ParseSnapshotData(
    const client::SnapshotPacket &snapshot) {
    std::vector<ParsedEntity> entities;

    const size_t kEntityStateSize = 16;  // 16 bytes per EntityState

    // Check if payload contains WorldSnapshotPacket format (header + entities)
    // or just a single EntityState (current server implementation)
    if (snapshot.payload_size == kEntityStateSize) {
        // Single EntityState format (12 bytes) - current server implementation
        ParsedEntity entity;

        // EntityState structure:
        // - entity_id (u32, 4 bytes)
        // - entity_type (u8, 1 byte)
        // - reserved (u8, 1 byte)
        // - pos_x (u16, 2 bytes)
        // - pos_y (u16, 2 bytes)
        // - angle (u16, 2 bytes)
        // - velocity_x (u16, 2 bytes)
        // - velocity_y (u16, 2 bytes)

        entity.entity_id = static_cast<uint32_t>(snapshot.payload[0]) |
                           (static_cast<uint32_t>(snapshot.payload[1]) << 8) |
                           (static_cast<uint32_t>(snapshot.payload[2]) << 16) |
                           (static_cast<uint32_t>(snapshot.payload[3]) << 24);

        entity.entity_type = snapshot.payload[4];
        // snapshot.payload[5] is reserved byte, skip it

        entity.pos_x = static_cast<uint16_t>(snapshot.payload[6]) |
                       (static_cast<uint16_t>(snapshot.payload[7]) << 8);

        entity.pos_y = static_cast<uint16_t>(snapshot.payload[8]) |
                       (static_cast<uint16_t>(snapshot.payload[9]) << 8);

        entity.angle = static_cast<uint16_t>(snapshot.payload[10]) |
                       (static_cast<uint16_t>(snapshot.payload[11]) << 8);

        entity.velocity_x = static_cast<uint16_t>(snapshot.payload[12]) |
                            (static_cast<uint16_t>(snapshot.payload[13]) << 8);

        entity.velocity_y = static_cast<uint16_t>(snapshot.payload[14]) |
                            (static_cast<uint16_t>(snapshot.payload[15]) << 8);

        entities.push_back(entity);
    } else if (snapshot.payload_size >= 4) {
        // WorldSnapshotPacket format (with header)
        uint16_t entity_count =
            static_cast<uint16_t>(snapshot.payload[0]) |
            (static_cast<uint16_t>(snapshot.payload[1]) << 8);

        size_t offset = 4;  // Skip header (entity_count + 2 reserved)

        // Parse each entity (12 bytes each)
        for (uint16_t i = 0; i < entity_count && offset + kEntityStateSize <=
                                                     snapshot.payload_size;
            ++i) {
            ParsedEntity entity;

            entity.entity_id =
                static_cast<uint32_t>(snapshot.payload[offset + 0]) |
                (static_cast<uint32_t>(snapshot.payload[offset + 1]) << 8) |
                (static_cast<uint32_t>(snapshot.payload[offset + 2]) << 16) |
                (static_cast<uint32_t>(snapshot.payload[offset + 3]) << 24);

            entity.entity_type = snapshot.payload[offset + 4];
            // offset + 5 is reserved byte, skip it

            entity.pos_x =
                static_cast<uint16_t>(snapshot.payload[offset + 6]) |
                (static_cast<uint16_t>(snapshot.payload[offset + 7]) << 8);

            entity.pos_y =
                static_cast<uint16_t>(snapshot.payload[offset + 8]) |
                (static_cast<uint16_t>(snapshot.payload[offset + 9]) << 8);

            entity.angle =
                static_cast<uint16_t>(snapshot.payload[offset + 10]) |
                (static_cast<uint16_t>(snapshot.payload[offset + 11]) << 8);

            entity.velocity_x =
                static_cast<uint16_t>(snapshot.payload[offset + 12]) |
                (static_cast<uint16_t>(snapshot.payload[offset + 13]) << 8);

            entity.velocity_y =
                static_cast<uint16_t>(snapshot.payload[offset + 14]) |
                (static_cast<uint16_t>(snapshot.payload[offset + 15]) << 8);

            entities.push_back(entity);
            offset += kEntityStateSize;
        }
    }

    return entities;
}

/**
 * @brief Display snapshot data (for debugging).
 *
 * @param snapshot The snapshot packet received from server
 */
static void DisplaySnapshotData(const client::SnapshotPacket &snapshot) {
    static int last_tick = -1;
    static int display_count = 0;

    last_tick = snapshot.tick;
    display_count++;

    std::cout << "\n[UDP Snapshot] Tick=" << snapshot.tick
              << " PayloadSize=" << snapshot.payload_size << " bytes"
              << std::endl;

    // Display raw payload bytes (first 12 bytes)
    if (display_count <= 3 && snapshot.payload_size > 0) {
        std::cout << "  Raw payload (hex): ";
        for (size_t i = 0; i < std::min<size_t>(snapshot.payload_size, 12);
            ++i) {
            std::cout << std::hex << std::setfill('0') << std::setw(2)
                      << static_cast<int>(snapshot.payload[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    // Parse entities using the new function
    auto entities = ParseSnapshotData(snapshot);

    std::cout << "  Entities: " << entities.size() << std::endl;

    for (size_t i = 0; i < entities.size(); ++i) {
        const auto &entity = entities[i];
        std::cout << "    [" << i << "] ID=" << entity.entity_id << " Type=0x"
                  << std::hex << static_cast<int>(entity.entity_type)
                  << std::dec << " Pos=(" << entity.pos_x << ","
                  << entity.pos_y << ") Angle=" << entity.angle << " Vel=("
                  << entity.velocity_x << "," << entity.velocity_y << ")"
                  << std::endl;
    }
    std::cout << std::flush;
}

static void CreateNewEntity(GameWorld &game_world,
    const ParsedEntity &entity_data, std::optional<size_t> &entity_index) {
    auto new_entity = game_world.registry_.SpawnEntity();
    if (entity_data.entity_type == 0x00) {
        // Player entity
        bool is_local = false;
        if (game_world.server_connection_ &&
            game_world.server_connection_->is_connected()) {
            uint32_t controlled =
                game_world.server_connection_->controlled_entity_id();
            if (controlled != 0 && controlled == entity_data.entity_id)
                is_local = true;
        }

        FactoryActors::GetInstance().CreateActor(
            new_entity, game_world.registry_, "player", is_local);
    } else {
        printf("[Snapshot] Unknown entity type 0x%02X for entity ID %u\n",
            entity_data.entity_type, entity_data.entity_id);
    }

    game_world.registry_.AddComponent<Component::NetworkId>(new_entity,
        Component::NetworkId{static_cast<int>(entity_data.entity_id)});
    size_t entity_id = new_entity.GetId();
    entity_index = entity_id;

    std::cout << "[Snapshot] Created entity #" << entity_id
              << " for NetworkId=" << entity_data.entity_id << std::endl;
}

static void UpdateExistingEntityTransform(GameWorld &game_world,
    size_t entity_index, const ParsedEntity &entity_data) {
    // Update existing entity's transform
    if (!game_world.registry_.GetComponents<Component::Transform>().has(
            entity_index))
        return;
    auto &transform = game_world.registry_
                          .GetComponents<Component::Transform>()[entity_index];
    if (transform.has_value()) {
        transform->x = static_cast<float>(entity_data.pos_x);
        transform->y = static_cast<float>(entity_data.pos_y);
        transform->rotationDegrees =
            static_cast<float>(entity_data.angle / 10.0f);
    }
    try {
        auto &velocity =
            game_world.registry_
                .GetComponents<Component::Velocity>()[entity_index];
        if (velocity.has_value()) {
            // Decode velocity from bias encoding: [0, 65535] -> [-32768,
            // 32767]
            velocity->vx = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_x) - 32768);
            velocity->vy = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_y) - 32768);
        }
    } catch (const std::exception &e) {
        // Velocity component might not exist; ignore if so
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
static void ApplySnapshotToRegistry(
    GameWorld &game_world, const client::SnapshotPacket &snapshot) {
    auto entities = ParseSnapshotData(snapshot);

    auto &net_ids = game_world.registry_.GetComponents<Component::NetworkId>();

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
            CreateNewEntity(game_world, entity_data, entity_index);
        } else {
            UpdateExistingEntityTransform(
                game_world, entity_index.value(), entity_data);
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

        game_world.server_connection_->ConnectToServer(config.username);

        // Poll io_context to process connection attempt
        for (int i = 0; i < kPollIterations &&
                        !game_world.server_connection_->is_connected();
            ++i) {
            game_world.io_context_.poll();
            sf::sleep(sf::milliseconds(kPollDelayMs));
        }

        connected = game_world.server_connection_->is_connected();

        if (!connected && retry < kMaxRetries - 1) {
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
        sf::Event event;
        while (game_world.window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                game_world.window_.close();
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

                // packet_count++;
                //  if (packet_count <= 5 || packet_count % 60 == 0) {
                //      std::cout << "[Client] Received UDP snapshot #"
                //                << packet_count << " (tick=" <<
                //                snapshot->tick
                //                << ")\n";
                //  }

                // Apply snapshot data to registry entities
                ApplySnapshotToRegistry(game_world, *snapshot);

                // Display snapshot data for debugging (optional)
                // DisplaySnapshotData(*snapshot);
            }
        }

        // Calculate delta time at the beginning of the frame
        game_world.last_delta_ =
            game_world.delta_time_clock_.restart().asSeconds();

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
