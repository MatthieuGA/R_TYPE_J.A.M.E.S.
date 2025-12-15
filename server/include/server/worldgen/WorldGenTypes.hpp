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
 */
struct LevelDefinition {
    std::string name;
    std::string author;
    std::vector<std::string> frames;  ///< Ordered WGF UUIDs
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
