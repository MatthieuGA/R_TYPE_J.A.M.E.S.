#include <cmath>
#include <iostream>
#include <limits>
#include <map>

#include "server/CoreComponents.hpp"
#include "server/GameWorldDatas.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {
/**
 * @brief Straight movement function.
 *
 * @param reg The registry.
 * @param entityId The ID of the entity.
 * @param transform The Transform component of the entity.
 * @param velocity The Velocity component of the entity.
 * @param patternMovement The PatternMovement component of the entity.
 * @param dt Delta time (seconds).
 */
void StraightMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
    // Straight movement logic
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vx < 0 && transform.x < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
    if (velocity.vy < 0 && transform.y < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void SineHorizontalMovementFunction(Engine::registry &reg,
    std::size_t entityId, Component::Transform &transform,
    Component::Velocity &velocity, Component::PatternMovement &patternMovement,
    float dt) {
    // Sine horizontal movement logic
    float sineOffset =
        patternMovement.amplitude.y *
        std::sin(patternMovement.frequency.y * patternMovement.elapsed);
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = sineOffset;

    if (velocity.vx < 0 && transform.x < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void ZigZagHorizontalMovementFunction(Engine::registry &reg,
    std::size_t entityId, Component::Transform &transform,
    Component::Velocity &velocity, Component::PatternMovement &patternMovement,
    float dt) {
    // Zig-zag horizontal movement logic
    float sineOffset =
        patternMovement.amplitude.y *
        std::sin(patternMovement.frequency.y * patternMovement.elapsed);
    velocity.vx = patternMovement.baseDir.x * patternMovement.baseSpeed;
    velocity.vy = sineOffset > 0 ? patternMovement.amplitude.y
                                 : -patternMovement.amplitude.y;

    if (velocity.vx < 0 && transform.x < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void SineVerticalMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
    // Sine vertical movement logic
    float sineOffset =
        patternMovement.amplitude.x *
        std::sin(patternMovement.frequency.x * patternMovement.elapsed);
    velocity.vx = sineOffset;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vy < 0 && transform.y < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void ZigZagVerticalMovementFunction(Engine::registry &reg,
    std::size_t entityId, Component::Transform &transform,
    Component::Velocity &velocity, Component::PatternMovement &patternMovement,
    float dt) {
    // Zig-zag vertical movement logic
    float sineOffset =
        patternMovement.amplitude.x *
        std::sin(patternMovement.frequency.x * patternMovement.elapsed);
    velocity.vx = sineOffset > 0 ? patternMovement.amplitude.x
                                 : -patternMovement.amplitude.x;
    velocity.vy = patternMovement.baseDir.y * patternMovement.baseSpeed;

    if (velocity.vy < 0 && transform.y < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void WaveMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
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
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
    if (velocity.vy < 0 && transform.y < -100.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
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
 */
void WaypointsMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
    // Waypoints movement logic
    if (patternMovement.waypoints.empty())
        return;
    if (patternMovement.currentWaypoint >= patternMovement.waypoints.size())
        patternMovement.currentWaypoint = 0;
    vector2f targetWaypoint =
        patternMovement.waypoints[patternMovement.currentWaypoint];
    vector2f direction = {
        targetWaypoint.x - transform.x, targetWaypoint.y - transform.y};
    float distance =
        std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance < patternMovement.waypointThreshold) {
        patternMovement.currentWaypoint =
            (patternMovement.currentWaypoint + 1) %
            patternMovement.waypoints.size();
        WaypointsMovementFunction(
            reg, entityId, transform, velocity, patternMovement, dt);
        return;
    }
    if (distance > 0.0f) {
        direction.x /= distance;
        direction.y /= distance;
        velocity.vx = direction.x * patternMovement.baseSpeed;
        velocity.vy = direction.y * patternMovement.baseSpeed;
    }
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
 */
void FollowPlayerMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
    // Follow player movement logic
    try {
        if (patternMovement.targetEntityId == -1) {
            // Find the closest player entity
            size_t id_min = -1;
            float dist_min = std::numeric_limits<float>::max();

            auto &playerTags = reg.GetComponents<Component::PlayerTag>();
            auto &playerTransform = reg.GetComponents<Component::Transform>();
            for (auto &&[i, playerTag, pTransform] :
                make_indexed_zipper(playerTags, playerTransform)) {
                vector2f direction = {
                    pTransform.x - transform.x, pTransform.y - transform.y};
                float distance = std::sqrt(
                    direction.x * direction.x + direction.y * direction.y);
                if (distance > 0.0f && distance < dist_min) {
                    dist_min = distance;
                    id_min = i;
                }
            }
            if (id_min == -1)
                return;
            // Set the target entity ID to the closest player
            patternMovement.targetEntityId = id_min;
        } else {
            // Move towards the target player entity
            try {
                auto &targetTransform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(patternMovement.targetEntityId));

                vector2f direction = {targetTransform.x - transform.x,
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
                return;
            }
        }
    } catch (const std::exception &e) {
        return;
    }
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
 */
void CircularMovementFunction(Engine::registry &reg, std::size_t entityId,
    Component::Transform &transform, Component::Velocity &velocity,
    Component::PatternMovement &patternMovement, float dt) {
    // Circular movement logic
    patternMovement.angle =
        (patternMovement.baseSpeed / patternMovement.radius) *
        patternMovement.elapsed;
    patternMovement.angle = std::fmod(patternMovement.angle, 2 * 3.14159265f);

    transform.x = patternMovement.spawnPos.x +
                  patternMovement.radius * std::cos(patternMovement.angle);
    transform.y = patternMovement.spawnPos.y +
                  patternMovement.radius * std::sin(patternMovement.angle);
}

/**
 * @brief Get the movement function corresponding to the pattern type.
 *
 * @param type The PatternMovement type.
 * @return A function that applies the movement logic for the given type.
 */
std::function<void(Engine::registry &, std::size_t, Component::Transform &,
    Component::Velocity &, Component::PatternMovement &, float)>
GetNextMovementFunction(Component::PatternMovement::PatternType type) {
    std::map<Component::PatternMovement::PatternType,
        std::function<void(Engine::registry &, std::size_t,
            Component::Transform &, Component::Velocity &,
            Component::PatternMovement &, float)>>
        movementFunctionMap = {
            {Component::PatternMovement::PatternType::Straight,
                StraightMovementFunction},
            {Component::PatternMovement::PatternType::SineHorizontal,
                SineHorizontalMovementFunction},
            {Component::PatternMovement::PatternType::SineVertical,
                SineVerticalMovementFunction},
            {Component::PatternMovement::PatternType::Wave,
                WaveMovementFunction},
            {Component::PatternMovement::PatternType::ZigZagHorizontal,
                ZigZagHorizontalMovementFunction},
            {Component::PatternMovement::PatternType::ZigZagVertical,
                ZigZagVerticalMovementFunction},
            {Component::PatternMovement::PatternType::Waypoints,
                WaypointsMovementFunction},
            {Component::PatternMovement::PatternType::FollowPlayer,
                FollowPlayerMovementFunction},
            {Component::PatternMovement::PatternType::Circular,
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
void PaternMovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities,
    Engine::sparse_array<Component::PatternMovement> &patern_movements) {
    for (auto &&[i, transform, velocity, patern_movement] :
        make_indexed_zipper(transforms, velocities, patern_movements)) {
        patern_movement.elapsed += TICK_RATE_SECONDS;

        try {
            GetNextMovementFunction(patern_movement.type)(reg, i, transform,
                velocity, patern_movement, TICK_RATE_SECONDS);
        } catch (const std::exception &e) {
            continue;
        }
    }
}
}  // namespace server
