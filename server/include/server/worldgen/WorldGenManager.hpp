/**
 * @file WorldGenManager.hpp
 * @brief Runtime manager for deterministic world generation.
 *
 * The WorldGenManager is the main interface for world generation during
 * gameplay. It handles:
 * - Seed-based initialization for deterministic generation
 * - Frame selection based on difficulty and spawn rules
 * - Spawn event generation for the server to consume
 * - Level loading for fixed/editor-created levels
 * - State management for save/load functionality
 *
 * This class does NOT directly create ECS entities. It produces SpawnEvents
 * that the server interprets to create actual game objects.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "server/worldgen/DeterministicRNG.hpp"
#include "server/worldgen/WorldGenConfigLoader.hpp"
#include "server/worldgen/WorldGenTypes.hpp"

namespace worldgen {

/**
 * @brief Callback type for spawn event notifications.
 */
using SpawnEventCallback = std::function<void(const SpawnEvent &)>;

/**
 * @brief Runtime manager for deterministic world generation.
 *
 * The WorldGenManager orchestrates frame selection and spawn event generation
 * using a deterministic PRNG. It supports both endless mode (procedural
 * selection) and fixed levels (predetermined frame sequences).
 *
 * Example usage:
 * @code
 * // Setup
 * WorldGenConfigLoader loader;
 * loader.LoadFromDirectories("core", "user");
 *
 * WorldGenManager manager(loader);
 * manager.SetSpawnCallback([](const SpawnEvent& event) {
 *     // Create entity from event
 * });
 *
 * // Start endless mode
 * manager.InitializeEndless(12345, 3.0f);
 *
 * // In game loop
 * manager.Update(delta_time, scroll_speed);
 *
 * // Or for fixed levels
 * manager.InitializeLevel("level-uuid");
 * @endcode
 */
class WorldGenManager {
 public:
    /**
     * @brief Constructs the manager with a reference to the config loader.
     *
     * @param config_loader Reference to a loaded WorldGenConfigLoader.
     */
    explicit WorldGenManager(const WorldGenConfigLoader &config_loader);

    ~WorldGenManager() = default;

    // Non-copyable, movable
    WorldGenManager(const WorldGenManager &) = delete;
    WorldGenManager &operator=(const WorldGenManager &) = delete;
    WorldGenManager(WorldGenManager &&) = default;
    WorldGenManager &operator=(WorldGenManager &&) = default;

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * @brief Initializes endless mode with a seed.
     *
     * Creates a SeedMetadata capturing the current WGF library state and
     * begins procedural generation based on difficulty scaling.
     *
     * @param seed The seed value for deterministic generation.
     * @param initial_difficulty Starting difficulty level [0, 10].
     * @return True if initialization succeeded.
     */
    bool InitializeEndless(uint64_t seed, float initial_difficulty = 1.0f);

    /**
     * @brief Initializes endless mode with auto-generated seed.
     *
     * @param initial_difficulty Starting difficulty level [0, 10].
     * @return The generated seed value.
     */
    uint64_t InitializeEndlessRandom(float initial_difficulty = 1.0f);

    /**
     * @brief Initializes a fixed level by UUID.
     *
     * Loads the level definition and prepares to play through its
     * predetermined frame sequence.
     *
     * @param level_uuid The UUID of the level to load.
     * @return True if the level was found and loaded.
     */
    bool InitializeLevel(const std::string &level_uuid);

    /**
     * @brief Initializes from a complete SeedMetadata structure.
     *
     * Used for loading saved games or sharing seeds between players.
     *
     * @param metadata The complete seed metadata to use.
     * @return True if initialization succeeded.
     */
    bool InitializeFromMetadata(const SeedMetadata &metadata);

    /**
     * @brief Resets the worldgen to initial state without changing the seed.
     *
     * Useful for restarting a level from the beginning.
     */
    void Reset();

    /**
     * @brief Stops world generation.
     */
    void Stop();

    // ========================================================================
    // Runtime Operations
    // ========================================================================

    /**
     * @brief Updates the world generation state.
     *
     * Call this every tick to advance world generation. The manager tracks
     * the world scroll position and generates spawn events as needed.
     *
     * @param delta_time Time elapsed since last update (seconds).
     * @param scroll_speed World scroll speed (units per second).
     */
    void Update(float delta_time, float scroll_speed);

    /**
     * @brief Gets the next spawn event without consuming it.
     *
     * @return The next event, or nullopt if none pending.
     */
    std::optional<SpawnEvent> PeekNextEvent() const;

    /**
     * @brief Gets and removes the next spawn event.
     *
     * @return The next event, or nullopt if none pending.
     */
    std::optional<SpawnEvent> PopNextEvent();

    /**
     * @brief Checks if there are pending spawn events.
     *
     * @return True if events are waiting to be processed.
     */
    bool HasPendingEvents() const;

    /**
     * @brief Sets the callback for spawn events.
     *
     * If set, this callback is invoked for each spawn event as they are
     * generated, in addition to queueing them.
     *
     * @param callback The callback function.
     */
    void SetSpawnCallback(SpawnEventCallback callback);

    /**
     * @brief Clears the spawn callback.
     *
     * Should be called before the system that owns the callback is destroyed
     * to prevent use-after-free issues.
     */
    void ClearCallback();

    /**
     * @brief Manually advances to the next frame.
     *
     * Useful for debugging or level editor preview.
     *
     * @return True if a frame was available.
     */
    bool AdvanceFrame();

    // ========================================================================
    // State Queries
    // ========================================================================

    /**
     * @brief Gets the current worldgen state.
     *
     * @return Const reference to the current state.
     */
    const WorldGenState &GetState() const;

    /**
     * @brief Gets the seed metadata.
     *
     * @return Const reference to the seed metadata.
     */
    const SeedMetadata &GetSeedMetadata() const;

    /**
     * @brief Gets the current frame index.
     *
     * @return The current frame number (0-indexed).
     */
    int GetCurrentFrameIndex() const;

    /**
     * @brief Gets the current world offset (scroll position).
     *
     * @return The current X offset in world units.
     */
    float GetWorldOffset() const;

    /**
     * @brief Gets the current difficulty level.
     *
     * @return The current difficulty [0, 10].
     */
    float GetCurrentDifficulty() const;

    /**
     * @brief Checks if worldgen is currently active.
     *
     * @return True if generation is running.
     */
    bool IsActive() const;

    /**
     * @brief Checks if the level is complete (fixed mode only).
     *
     * @return True if all frames have been played in fixed mode.
     */
    bool IsLevelComplete() const;

    /**
     * @brief Checks if running in endless mode.
     *
     * @return True if endless mode, false if fixed level.
     */
    bool IsEndlessMode() const;

    /**
     * @brief Gets the current WGF definition being played.
     *
     * @return Pointer to current WGF, or nullptr if none.
     */
    const WGFDefinition *GetCurrentWGF() const;

    // ========================================================================
    // Level Management
    // ========================================================================

    /**
     * @brief Loads a level definition from a JSON file.
     *
     * @param filepath Path to the level JSON file.
     * @return True if loaded successfully.
     */
    bool LoadLevelFromFile(const std::string &filepath);

    /**
     * @brief Loads a level from a JSON string.
     *
     * @param json_content The JSON content as a string.
     * @return True if parsed and loaded successfully.
     */
    bool LoadLevelFromString(const std::string &json_content);

    /**
     * @brief Adds a level definition to the manager.
     *
     * @param level The level definition to add.
     */
    void AddLevel(const LevelDefinition &level);

    /**
     * @brief Gets a level definition by UUID.
     *
     * @param uuid The level UUID.
     * @return Pointer to the level, or nullptr if not found.
     */
    const LevelDefinition *GetLevelByUUID(const std::string &uuid) const;

    /**
     * @brief Gets all loaded level definitions.
     *
     * @return Const reference to the vector of levels.
     */
    const std::vector<LevelDefinition> &GetAllLevels() const;

    // ========================================================================
    // Save/Load Support
    // ========================================================================

    /**
     * @brief Saves the current state for later restoration.
     *
     * @return The complete worldgen state.
     */
    WorldGenState SaveState() const;

    /**
     * @brief Restores a previously saved state.
     *
     * @param state The state to restore.
     * @return True if restoration succeeded.
     */
    bool RestoreState(const WorldGenState &state);

    // ========================================================================
    // Logging
    // ========================================================================

    /**
     * @brief Sets the logging callback.
     *
     * @param callback Function to receive log messages.
     */
    void SetLogCallback(LogCallback callback);

 private:
    /**
     * @brief Selects the next WGF for endless mode.
     *
     * Uses the PRNG to select a frame based on difficulty weighting
     * and spawn rules.
     *
     * @return UUID of the selected WGF, or empty string if none available.
     */
    std::string SelectNextWGF();

    /**
     * @brief Generates spawn events for a WGF frame.
     *
     * @param wgf The WGF to generate events from.
     * @param frame_start_x The world X position where this frame starts.
     */
    void GenerateFrameEvents(const WGFDefinition &wgf, float frame_start_x);

    /**
     * @brief Calculates the difficulty weight for a WGF.
     *
     * @param wgf The WGF to evaluate.
     * @return The selection weight (higher = more likely).
     */
    float CalculateDifficultyWeight(const WGFDefinition &wgf) const;

    /**
     * @brief Checks if a WGF can be selected based on spawn rules.
     *
     * @param wgf The WGF to check.
     * @return True if the WGF can be selected.
     */
    bool CanSelectWGF(const WGFDefinition &wgf) const;

    /**
     * @brief Updates difficulty based on progression.
     */
    void UpdateDifficulty();

    /**
     * @brief Logs a message using the configured callback.
     *
     * @param level Log level.
     * @param message Message to log.
     */
    void Log(LogLevel level, const std::string &message);

    // ========================================================================
    // Member Variables
    // ========================================================================

    const WorldGenConfigLoader &config_loader_;
    DeterministicRNG rng_;
    WorldGenState state_;
    std::queue<SpawnEvent> event_queue_;
    SpawnEventCallback spawn_callback_;
    LogCallback log_callback_;

    std::vector<LevelDefinition> levels_;
    std::unordered_map<std::string, size_t> level_uuid_to_index_;

    // Frame history for spawn rules
    std::vector<std::string> recent_frame_uuids_;
    static constexpr size_t kMaxFrameHistory = 10;

    // Current frame state
    std::string current_wgf_uuid_;
    float current_frame_end_x_ = 0.0f;
    int next_frame_in_level_ = 0;  // For fixed level mode
};

}  // namespace worldgen
