#include <cmath>
#include <limits>
#include <map>
#include <vector>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Straight movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool StraightMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Straight movement logic
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vx < 0 && transform.x < -100.f) {
        return true;
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        return true;
    }
    if (velocity.vy < 0 && transform.y < -100.f) {
        return true;
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        return true;
    }
    return false;
}

/**
 * @brief Sine horizontal movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool SineHorizontalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Sine horizontal movement logic
    float sineOffset =
        patternMovement.amplitude.y *
        std::sin(patternMovement.frequency.y * patternMovement.elapsed);
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = sineOffset;

    if (velocity.vx < 0 && transform.x < -100.f) {
        return true;
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        return true;
    }
    return false;
}

/**
 * @brief Zig-zag horizontal movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool ZigZagHorizontalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Zig-zag horizontal movement logic
    float sineOffset =
        patternMovement.amplitude.y *
        std::sin(patternMovement.frequency.y * patternMovement.elapsed);
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = sineOffset > 0 ? patternMovement.amplitude.y
                                 : -patternMovement.amplitude.y;

    if (velocity.vx < 0 && transform.x < -100.f) {
        return true;
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        return true;
    }
    return false;
}

/**
 * @brief Sine vertical movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool SineVerticalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Sine vertical movement logic
    float sineOffset =
        patternMovement.amplitude.x *
        std::sin(patternMovement.frequency.x * patternMovement.elapsed);
    velocity.vx = sineOffset;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vy < 0 && transform.y < -100.f) {
        return true;
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        return true;
    }
    return false;
}

/**
 * @brief Zig-zag vertical movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool ZigZagVerticalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Zig-zag vertical movement logic
    float sineOffset =
        patternMovement.amplitude.x *
        std::sin(patternMovement.frequency.x * patternMovement.elapsed);
    velocity.vx = sineOffset > 0 ? patternMovement.amplitude.x
                                 : -patternMovement.amplitude.x;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vy < 0 && transform.y < -100.f) {
        return true;
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        return true;
    }
    return false;
}

/**
 * @brief Wave movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool WaveMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Wave movement logic
    float sineOffsetX =
        patternMovement.amplitude.x *
        std::sin(patternMovement.frequency.x * patternMovement.elapsed);
    float sineOffsetY =
        patternMovement.amplitude.y *
        std::sin(patternMovement.frequency.y * patternMovement.elapsed);
    velocity.vx =
        patternMovement.baseDir.x * patternMovement.baseSpeed + sineOffsetX;
    velocity.vy =
        patternMovement.baseDir.y * patternMovement.baseSpeed + sineOffsetY;

    if (velocity.vx < 0 && transform.x < -100.f) {
        return true;
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        return true;
    }
    if (velocity.vy < 0 && transform.y < -100.f) {
        return true;
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        return true;
    }
    return false;
}

/**
 * @brief Waypoints movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool WaypointsMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Waypoints movement logic
    if (patternMovement.waypoints.empty())
        return false;
    if (patternMovement.currentWaypoint >= patternMovement.waypoints.size())
        patternMovement.currentWaypoint = 0;
    auto targetWaypoint =
        patternMovement.waypoints[patternMovement.currentWaypoint];
    sf::Vector2f direction = {
        targetWaypoint.x - transform.x, targetWaypoint.y - transform.y};
    float distance =
        std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance < patternMovement.waypointThreshold) {
        patternMovement.currentWaypoint =
            (patternMovement.currentWaypoint + 1) %
            patternMovement.waypoints.size();
        return WaypointsMovementFunction(
            reg, entityId, transform, velocity, patternMovement, dt);
    }
    if (distance > 0.0f) {
        direction.x /= distance;
        direction.y /= distance;
        velocity.vx = direction.x * patternMovement.baseSpeed;
        velocity.vy = direction.y * patternMovement.baseSpeed;
    }
    return false;
}

/**
 * @brief Follow player movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool FollowPlayerMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Follow player movement logic
    try {
        if (patternMovement.targetEntityId == -1) {
            // Find the closest player entity
            size_t id_min = -1;
            float dist_min = std::numeric_limits<float>::max();

            auto &playerTags = reg.GetComponents<Com::PlayerTag>();
            auto &playerTransform = reg.GetComponents<Com::Transform>();
            for (auto &&[i, playerTag, pTransform] :
                make_indexed_zipper(playerTags, playerTransform)) {
                sf::Vector2f direction = {
                    pTransform.x - transform.x, pTransform.y - transform.y};
                float distance = std::sqrt(
                    direction.x * direction.x + direction.y * direction.y);
                if (distance > 0.0f && distance < dist_min) {
                    dist_min = distance;
                    id_min = i;
                }
            }
            if (id_min == -1)
                return false;
            // Set the target entity ID to the closest player
            patternMovement.targetEntityId = id_min;
        } else {
            // Move towards the target player entity
            try {
                auto &targetTransform = reg.GetComponent<Com::Transform>(
                    reg.EntityFromIndex(patternMovement.targetEntityId));

                sf::Vector2f direction = {targetTransform.x - transform.x,
                    targetTransform.y - transform.y};
                float distance = std::sqrt(
                    direction.x * direction.x + direction.y * direction.y);
                if (distance > 0.0f) {
                    direction.x /= distance;
                    direction.y /= distance;
                    velocity.vx = direction.x * patternMovement.baseSpeed;
                    velocity.vy = direction.y * patternMovement.baseSpeed;
                }
            } catch (const std::exception &e) {
                // Target entity not found, reset targetEntityId
                patternMovement.targetEntityId = -1;
                return false;
            }
        }
    } catch (const std::exception &e) {
        return false;
    }
    return false;
}

/**
 * @brief Circular movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 * @return true if the entity should be killed, false otherwise.
 */
bool CircularMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Circular movement logic
    patternMovement.angle =
        (patternMovement.baseSpeed / patternMovement.radius) *
        patternMovement.elapsed;
    patternMovement.angle = std::fmod(patternMovement.angle, 2 * 3.14159265f);

    transform.x = patternMovement.spawnPos.x +
                  patternMovement.radius * std::cos(patternMovement.angle);
    transform.y = patternMovement.spawnPos.y +
                  patternMovement.radius * std::sin(patternMovement.angle);
    return false;
}

/**
 * @brief Get the movement function corresponding to the pattern type.
 *
 * @param type The PatternMovement type.
 * @return A function that applies the movement logic for the given type.
 */
std::function<bool(Eng::registry &, std::size_t, Com::Transform &,
    Com::Velocity &, Com::PatternMovement &, float)>
GetNextMovementFunction(Com::PatternMovement::PatternType type) {
    std::map<Com::PatternMovement::PatternType,
        std::function<bool(Eng::registry &, std::size_t, Com::Transform &,
            Com::Velocity &, Com::PatternMovement &, float)>>
        movementFunctionMap = {
            {Com::PatternMovement::PatternType::Straight,
                StraightMovementFunction},
            {Com::PatternMovement::PatternType::SineHorizontal,
                SineHorizontalMovementFunction},
            {Com::PatternMovement::PatternType::SineVertical,
                SineVerticalMovementFunction},
            {Com::PatternMovement::PatternType::Wave, WaveMovementFunction},
            {Com::PatternMovement::PatternType::ZigZagHorizontal,
                ZigZagHorizontalMovementFunction},
            {Com::PatternMovement::PatternType::ZigZagVertical,
                ZigZagVerticalMovementFunction},
            {Com::PatternMovement::PatternType::Waypoints,
                WaypointsMovementFunction},
            {Com::PatternMovement::PatternType::FollowPlayer,
                FollowPlayerMovementFunction},
            {Com::PatternMovement::PatternType::Circular,
                CircularMovementFunction},
        };
    if (movementFunctionMap.find(type) == movementFunctionMap.end())
        throw std::runtime_error("Unknown PatternMovement type");
    return movementFunctionMap[type];
}

/**
 * @brief System to apply pattern-based movement to entities.
 *
 * Updates entity velocities and positions based on their assigned movement
 * patterns.
 *
 * @param reg Engine registry (unused)
 * @param dt Delta time (seconds)
 * @param transforms Sparse array of Transform components
 * @param velocities Sparse array of Velocity components
 * @param patern_movements Sparse array of PatternMovement components
 */
void PaternMovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PatternMovement> &patern_movements) {
    // Collect entities to kill to avoid modifying sparse arrays during
    // iteration
    std::vector<Engine::entity> entities_to_kill;

    for (auto &&[i, transform, velocity, patern_movement] :
        make_indexed_zipper(transforms, velocities, patern_movements)) {
        patern_movement.elapsed += dt;

        try {
            bool should_kill = GetNextMovementFunction(patern_movement.type)(
                reg, i, transform, velocity, patern_movement, dt);
            if (should_kill) {
                entities_to_kill.push_back(reg.EntityFromIndex(i));
            }
        } catch (const std::exception &e) {
            continue;
        }
    }

    // Now kill entities after iteration is complete
    for (Engine::entity entity : entities_to_kill) {
        reg.KillEntity(entity);
    }
}
}  // namespace Rtype::Client
