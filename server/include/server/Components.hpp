#pragma once
#include <cstdint>
#include <string>

namespace server::Component {

/**
 * @brief Position component for entities in 2D space
 */
struct Position {
    float x;
    float y;
};

/**
 * @brief Velocity component for moving entities
 */
struct Velocity {
    float vx;
    float vy;
};

/**
 * @brief Health component for entities that can take damage
 */
struct Health {
    int current;
    int max;
};

/**
 * @brief Network ID component to identify entities across network
 */
struct NetworkId {
    uint32_t id;
};

/**
 * @brief Player component to mark entities controlled by players
 */
struct Player {
    uint32_t player_id;
    std::string name;
};

/**
 * @brief Enemy component to mark hostile entities
 */
struct Enemy {
    int damage;
    int points;  // Points awarded when destroyed
};

}  // namespace server::Component
