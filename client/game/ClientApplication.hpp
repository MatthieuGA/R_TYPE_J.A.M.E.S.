#pragma once
#include <vector>

#include "engine/GameWorld.hpp"
#include "game/CommandLineParser.hpp"

namespace Rtype::Client {

/**
 * @brief Application-level functions for R-Type client initialization and
 * execution.
 */
class ClientApplication {
 public:
    /**
     * @brief Represents a parsed entity from a snapshot.
     *
     * Stores entity state information received from server.
     */
    struct ParsedEntity {
        enum ProjectileType : uint8_t {
            kPlayerProjectile = 0x00,
            kPlayerChargedProjectile = 0x01,
            kMermaidProjectile = 0x02,
            kDaemonProjectile = 0x03
        };

        enum EntityType : uint8_t {
            kPlayerEntity = 0x00,
            kEnemyEntity = 0x01,
            kProjectileEntity = 0x02
        };

        enum EnemyType : uint8_t {
            kMermaidEnemy = 0x00,
            kKamiFishEnemy = 0x01,
            kDaemonEnemy = 0x02,
            kPowerUpInvinsibility = 0x04,
        };

        uint32_t entity_id;
        uint8_t entity_type;
        // Players :
        uint16_t pos_x;
        uint16_t pos_y;
        uint16_t angle;
        uint16_t velocity_x;
        uint16_t velocity_y;
        uint16_t health;
        // Projectiles :
        uint8_t projectile_type;
        // Enemies :
        uint8_t enemy_type;
        uint8_t current_animation;
        uint8_t current_frame;
    };

    /**
     * @brief Connect to the server with automatic retry mechanism.
     *
     * Attempts to establish a connection to the server with multiple retries.
     * Provides user feedback during the connection process.
     *
     * @param game_world The game world containing the server connection
     * @param config Client configuration with server details
     * @return true if connection succeeded, false otherwise
     */
    static bool ConnectToServerWithRetry(
        GameWorld &game_world, const ClientConfig &config);

    /**
     * @brief Run the main game loop.
     *
     * Handles event processing, game logic updates, and rendering until the
     * window is closed.
     *
     * @param game_world The game world to run
     */
    static void RunGameLoop(GameWorld &game_world);

    /**
     * @brief Initialize the client application.
     *
     * Sets up the game world, registry, and initial scene.
     *
     * @param game_world The game world to initialize
     */
    static void InitializeApplication(GameWorld &game_world);

 private:
    // Connection retry constants
    static constexpr int kMaxRetries = 3;
    static constexpr int kPollIterations = 20;
    static constexpr int kPollDelayMs = 100;
    static constexpr int kRetryDelayMs = 500;

    static void ApplySnapshotToRegistry(
        GameWorld &game_world, const client::SnapshotPacket &snapshot);
    static void CreateNewEntity(GameWorld &game_world, int tick,
        const ParsedEntity &entity_data, std::optional<size_t> &entity_index);
    static void UpdateExistingEntity(GameWorld &game_world,
        size_t entity_index, const ParsedEntity &entity_data);
    static std::vector<ParsedEntity> ParseSnapshotData(
        const client::SnapshotPacket &snapshot);
    static void DisplaySnapshotData(const client::SnapshotPacket &snapshot);
};

}  // namespace Rtype::Client
