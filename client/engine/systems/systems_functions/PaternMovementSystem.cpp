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

void SineVerticalMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Sine vertical movement logic
}

void WaveMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Wave movement logic
}

void WaypointsMovementFunction(Eng::registry &reg, std::size_t entityId,
    Com::Transform &transform, Com::Velocity &velocity,
    Com::PatternMovement &patternMovement, float dt) {
    // Waypoints movement logic
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
