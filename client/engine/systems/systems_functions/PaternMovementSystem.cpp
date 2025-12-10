#include <map>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

void StraightMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
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

void SineHorizontalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
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

void ZigZagHorizontalMovementFunction(Eng::registry &reg, std::size_t entityId,
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
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vx > 0 && transform.x > 2000.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
}

void SineVerticalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
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

void ZigZagVerticalMovementFunction(Eng::registry &reg, std::size_t entityId,
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
        reg.KillEntity(reg.EntityFromIndex(entityId));
    } else if (velocity.vy > 0 && transform.y > 1200.f) {
        reg.KillEntity(reg.EntityFromIndex(entityId));
    }
}

void WaveMovementFunction(Eng::registry &reg, std::size_t entityId,
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

void WaypointsMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Waypoints movement logic
    if (patternMovement.waypoints.empty())
        return;
    if (patternMovement.currentWaypoint >= patternMovement.waypoints.size())
        patternMovement.currentWaypoint = 0;
    sf::Vector2f targetWaypoint =
        patternMovement.waypoints[patternMovement.currentWaypoint];
    sf::Vector2f direction = {
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

void FollowPlayerMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Follow player movement logic
}

void CircularMovementFunction(Eng::registry &reg, std::size_t entityId,
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
}

std::function<void(Eng::registry &, std::size_t, Com::Transform &,
    Com::Velocity &, Com::PatternMovement &, float)>
GetNextMovementFunction(Com::PatternMovement::PatternType type) {
    std::map<Com::PatternMovement::PatternType,
        std::function<void(Eng::registry &, std::size_t, Com::Transform &,
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

void PaternMovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PatternMovement> &patern_movements) {
    for (auto &&[i, transform, velocity, patern_movement] :
        make_indexed_zipper(transforms, velocities, patern_movements)) {
        patern_movement.elapsed += dt;

        try {
            GetNextMovementFunction(patern_movement.type)(
                reg, i, transform, velocity, patern_movement, dt);
        } catch (const std::exception &e) {
            continue;
        }
    }
}
}  // namespace Rtype::Client
