/**
 * @file WorldGenSystem.hpp
 * @brief ECS system for integrating WorldGen with the game server.
 *
 * This system bridges the WorldGenManager (which produces SpawnEvents)
 * with the server's ECS (which creates actual entities).
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "include/registry.hpp"
#include "server/worldgen/WorldGen.hpp"

namespace server {

/**
 * @brief Manages world generation and spawns obstacles as ECS entities.
 *
 * This class integrates the worldgen system with the server's ECS.
 * It handles:
 * - Loading WGF configs and levels on startup
 * - Processing spawn events and creating entities
 * - Managing world scroll and difficulty progression
 *
 * Example usage:
 * @code
 * WorldGenSystem worldgen;
 * worldgen.Initialize("assets/worldgen");
 * worldgen.StartEndless(12345, 2.0f);
 *
 * // In game loop
 * worldgen.Update(delta_time, scroll_speed, registry);
 * @endcode
 */
class WorldGenSystem {
 public:
    WorldGenSystem();

    /**
     * @brief Construct with an existing WorldGenManager.
     *
     * @param manager Reference to initialized WorldGenManager
     * @param registry Reference to ECS registry
     */
    WorldGenSystem(
        worldgen::WorldGenManager &manager, Engine::registry &registry);

    ~WorldGenSystem() = default;

    /**
     * @brief Initializes the worldgen system.
     *
     * Loads WGF files from the specified base directory, which should
     * contain 'core/', 'user/', and optionally 'levels/' subdirectories.
     *
     * @param base_path Base path containing worldgen assets.
     * @return True if initialization succeeded.
     */
    bool Initialize(const std::string &base_path);

    /**
     * @brief Starts endless mode with the given seed.
     *
     * @param seed The seed for deterministic generation.
     * @param initial_difficulty Starting difficulty [0, 10].
     * @return True if started successfully.
     */
    bool StartEndless(uint64_t seed, float initial_difficulty = 2.0f);

    /**
     * @brief Starts endless mode with a random seed.
     *
     * @param initial_difficulty Starting difficulty [0, 10].
     * @return The generated seed value.
     */
    uint64_t StartEndlessRandom(float initial_difficulty = 2.0f);

    /**
     * @brief Starts a fixed level by name.
     *
     * @param level_name The level name to search for.
     * @return True if the level was found and started.
     */
    bool StartLevel(const std::string &level_name);

    /**
     * @brief Starts a level by UUID.
     *
     * @param level_uuid The level UUID.
     * @return True if started successfully.
     */
    bool StartLevelByUUID(const std::string &level_uuid);

    /**
     * @brief Updates world generation and processes spawn events.
     *
     * Call this every tick to advance worldgen and create entities.
     *
     * @param delta_time Time since last update (seconds).
     * @param scroll_speed World scroll speed (units per second).
     * @param registry The ECS registry for entity creation.
     */
    void Update(
        float delta_time, float scroll_speed, Engine::registry &registry);

    /**
     * @brief Stops world generation.
     */
    void Stop();

    /**
     * @brief Resets to the beginning of the current seed/level.
     */
    void Reset();

    /**
     * @brief Gets the current world scroll offset.
     *
     * @return Current X offset in world units.
     */
    float GetWorldOffset() const;

    /**
     * @brief Gets the current difficulty level.
     *
     * @return Current difficulty [0, 10].
     */
    float GetCurrentDifficulty() const;

    /**
     * @brief Gets the current frame index.
     *
     * @return Current frame number.
     */
    int GetCurrentFrame() const;

    /**
     * @brief Checks if worldgen is active.
     *
     * @return True if running.
     */
    bool IsActive() const;

    /**
     * @brief Checks if the level is complete (fixed mode).
     *
     * @return True if level ended.
     */
    bool IsLevelComplete() const;

    /**
     * @brief Checks if running in endless mode.
     *
     * @return True if endless, false if fixed level.
     */
    bool IsEndless() const;

    /**
     * @brief Gets the current seed value.
     *
     * @return The active seed.
     */
    uint64_t GetCurrentSeed() const;

    /**
     * @brief Loads all level files from the levels directory.
     *
     * @param levels_path Path to the levels directory.
     * @return Number of levels loaded.
     */
    int LoadLevels(const std::string &levels_path);

    /**
     * @brief Gets a list of available level names.
     *
     * @return Vector of level names.
     */
    std::vector<std::string> GetAvailableLevels() const;

    /**
     * @brief Gets access to the underlying WorldGenManager.
     *
     * @return Pointer to the manager, or nullptr if not initialized.
     */
    worldgen::WorldGenManager *GetManager();

    /**
     * @brief Gets access to the config loader.
     *
     * @return Pointer to the loader, or nullptr if not initialized.
     */
    worldgen::WorldGenConfigLoader *GetConfigLoader();

 private:
    /**
     * @brief Processes a spawn event and creates the corresponding entity.
     *
     * @param event The spawn event to process.
     * @param registry The ECS registry.
     */
    void ProcessSpawnEvent(
        const worldgen::SpawnEvent &event, Engine::registry &registry);

    /**
     * @brief Creates an obstacle entity from spawn event data.
     *
     * @param event The obstacle spawn event.
     * @param registry The ECS registry.
     */
    void SpawnObstacle(
        const worldgen::SpawnEvent &event, Engine::registry &registry);

    /**
     * @brief Creates an enemy entity from spawn event data using
     * FactoryActors.
     *
     * @param event The enemy spawn event.
     * @param registry The ECS registry.
     */
    void SpawnEnemy(
        const worldgen::SpawnEvent &event, Engine::registry &registry);

    std::unique_ptr<worldgen::WorldGenConfigLoader> config_loader_;
    std::unique_ptr<worldgen::WorldGenManager>
        manager_owned_;  // For Initialize() path
    worldgen::WorldGenManager *manager_ =
        nullptr;  // Pointer to manager (owned or external)
    Engine::registry *registry_ =
        nullptr;  // Pointer to registry (for spawn callback)
    bool initialized_ = false;
    float screen_width_ = 1920.0f;  // Screen width for despawn calculation
};

}  // namespace server
