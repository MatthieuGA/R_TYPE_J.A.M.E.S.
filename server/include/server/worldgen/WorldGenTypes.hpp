/**
 * @file WorldGenTypes.hpp
 * @brief Core data types for the WorldGen configuration system.
 *
 * This file defines all POD-like structures used by the WorldGen system,
 * including obstacle definitions, frame metadata, spawn rules, and seed data.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace worldgen {

/**
 * @brief Obstacle behavior types.
 */
enum class ObstacleType : uint8_t {
    kStatic = 0,        ///< Cannot be destroyed, blocks movement
    kDestructible = 1,  ///< Can be destroyed by player weapons
    kHazard = 2,        ///< Damages player on contact, may not block
    kDecoration = 3     ///< Visual only, no collision
};

/**
 * @brief 2D vector for positions and sizes.
 */
struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief 2D size structure.
 */
struct Size2f {
    float width = 32.0f;
    float height = 32.0f;
};

/**
 * @brief Collision configuration for an obstacle.
 */
struct CollisionData {
    bool enabled = true;
    int damage = 0;
};

/**
 * @brief Defines an enemy spawn within a WorldGen Frame.
 */
struct EnemySpawnData {
    std::string
        tag;  ///< Enemy tag for FactoryActors (e.g., "mermaid", "kamifish")
    Vec2f position;            ///< Position relative to frame start
    float spawn_delay = 0.0f;  ///< Delay from frame start in seconds
};

/**
 * @brief Defines a single obstacle within a WorldGen Frame.
 */
struct ObstacleData {
    ObstacleType type = ObstacleType::kStatic;
    std::string sprite;
    Vec2f position;
    Size2f size;
    CollisionData collision;
    int health = 0;  ///< 0 = indestructible (for destructible type)
};

/**
 * @brief A single parallax background layer.
 */
struct BackgroundLayer {
    std::string sprite;
    float parallax_factor = 1.0f;
    float scroll_speed = 1.0f;
};

/**
 * @brief Background configuration for a frame.
 */
struct BackgroundData {
    std::vector<BackgroundLayer> layers;
};

/**
 * @brief Rules controlling how often and when a frame can be selected.
 */
struct SpawnRules {
    int min_distance_from_last = 0;  ///< Minimum frames between uses
    float max_frequency = 1.0f;      ///< Maximum selection probability [0, 1]
    std::vector<std::string> requires_tags;  ///< Previous frame must have tags
};

/**
 * @brief Complete definition of a WorldGen Frame (WGF).
 *
 * A WGF represents one reusable segment of the game world, containing
 * obstacle layouts, background configuration, and metadata for selection.
 */
struct WGFDefinition {
    std::string uuid;                     ///< Unique identifier (UUIDv4)
    std::string name;                     ///< Human-readable name
    std::string description;              ///< Optional description
    float difficulty = 1.0f;              ///< Difficulty rating [0, 10]
    std::vector<std::string> tags;        ///< Category tags for filtering
    int width = 800;                      ///< Frame width in game units
    SpawnRules spawn_rules;               ///< Selection rules
    std::vector<ObstacleData> obstacles;  //< Obstacle definitions
    std::vector<EnemySpawnData> enemies;  ///< Enemy spawn definitions
    BackgroundData background;            ///< Background layers
    std::string source_file;  ///< Original file path (for debugging)
    bool is_core = true;      ///< True if from core/, false if user/
};

/**
 * @brief Difficulty scaling configuration.
 */
struct DifficultyScaling {
    float base = 1.0f;
    float per_frame = 0.05f;
    float max = 10.0f;
};

/**
 * @brief Endless mode specific configuration.
 */
struct EndlessModeConfig {
    float difficulty_increase_rate = 0.1f;
    float max_difficulty = 10.0f;
};

/**
 * @brief Global WorldGen configuration loaded from config.json.
 */
struct WorldGenConfig {
    int frame_width_default = 800;
    DifficultyScaling difficulty_scaling;
    EndlessModeConfig endless_mode;
};

/**
 * @brief Level definition for editor-created levels.
 *
 * A level is simply an ordered list of WGF UUIDs to play through.
 * This structure supports both the map editor and pre-designed levels.
 */
struct LevelDefinition {
    std::string uuid;                 ///< Unique identifier for the level
    std::string name;                 ///< Display name
    std::string author;               ///< Level creator
    std::string description;          ///< Optional description
    std::vector<std::string> frames;  ///< Ordered WGF UUIDs
    float target_difficulty = 5.0f;   ///< Target difficulty for the level
    bool is_endless = false;          ///< If true, continues after frames
};

/**
 * @brief Metadata embedded in a seed to ensure determinism forever.
 *
 * When a seed is used to generate a world, this metadata captures the
 * exact state of the WGF library at generation time. This ensures that
 * even if new WGFs are added later, the same seed produces the same world.
 *
 * For endless mode, the allowed_wgf_uuids list defines which WGFs can be
 * selected. For fixed levels, the frame sequence is predetermined.
 */
struct SeedMetadata {
    uint64_t seed_value = 0;         ///< The actual seed number
    float target_difficulty = 5.0f;  ///< Target difficulty for selection
    bool is_endless = true;          ///< Endless or fixed level mode
    std::vector<std::string>
        allowed_wgf_uuids;            ///< WGFs available at creation
    std::string level_uuid;           ///< Level UUID (if fixed mode)
    uint64_t creation_timestamp = 0;  ///< When the seed was created
};

/**
 * @brief Represents a spawn event generated by the worldgen system.
 *
 * A spawn event describes what should be spawned at a specific position.
 * The server uses these events to create actual game entities.
 */
struct SpawnEvent {
    enum class EventType : uint8_t {
        kObstacle = 0,    ///< Spawn an obstacle from WGF data
        kEnemy = 1,       ///< Spawn an enemy using FactoryActors
        kFrameStart = 2,  ///< Marks the beginning of a new WGF frame
        kFrameEnd = 3,    ///< Marks the end of a WGF frame
        kLevelEnd = 4     ///< Marks the end of the level (fixed mode only)
    };

    EventType type = EventType::kObstacle;
    std::string wgf_uuid;       ///< Source WGF UUID
    size_t obstacle_index = 0;  ///< Index into WGF's obstacles array
    float world_x = 0.0f;       ///< X position in world coordinates
    float world_y = 0.0f;       ///< Y position in world coordinates
    int frame_number = 0;       ///< Which frame this belongs to

    // Cached obstacle data for convenience (populated by WorldGenManager)
    ObstacleType obstacle_type = ObstacleType::kStatic;
    std::string sprite;
    Size2f size;
    CollisionData collision;
    int health = 0;

    // Enemy data (for kEnemy type)
    std::string enemy_tag;  ///< Enemy tag for FactoryActors (e.g., "mermaid",
                            ///< "kamifish")
};

/**
 * @brief Current state of the worldgen system.
 *
 * This structure captures the complete state of world generation,
 * allowing for save/load and deterministic replay.
 */
struct WorldGenState {
    SeedMetadata seed_metadata;
    int current_frame_index = 0;      ///< Current frame in the sequence
    float world_offset = 0.0f;        ///< Current world X offset (scroll)
    float current_difficulty = 1.0f;  ///< Current difficulty level
    uint64_t rng_state = 0;           ///< RNG state for save/load
    uint64_t rng_increment = 0;       ///< RNG increment for save/load
    std::string current_wgf_uuid;     ///< Currently active WGF UUID
    bool is_active = false;           ///< Is worldgen currently running
    bool level_complete = false;      ///< Has the level ended (fixed mode)
};

/**
 * @brief Converts a string to ObstacleType enum.
 *
 * @param str The string representation ("static", "destructible", etc.)
 * @return The corresponding ObstacleType, defaults to kStatic.
 */
inline ObstacleType StringToObstacleType(const std::string &str) {
    if (str == "destructible")
        return ObstacleType::kDestructible;
    if (str == "hazard")
        return ObstacleType::kHazard;
    if (str == "decoration")
        return ObstacleType::kDecoration;
    return ObstacleType::kStatic;
}

/**
 * @brief Converts an ObstacleType enum to string.
 *
 * @param type The ObstacleType value.
 * @return The string representation.
 */
inline std::string ObstacleTypeToString(ObstacleType type) {
    switch (type) {
        case ObstacleType::kDestructible:
            return "destructible";
        case ObstacleType::kHazard:
            return "hazard";
        case ObstacleType::kDecoration:
            return "decoration";
        default:
            return "static";
    }
}

}  // namespace worldgen
