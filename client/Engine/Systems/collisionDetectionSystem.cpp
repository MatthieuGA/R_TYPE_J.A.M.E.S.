#include <iostream>
#include <algorithm>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/Events/EngineEvent.hpp"
#include "Engine/originTool.hpp"


namespace Rtype::Client {
bool is_colliding_from_offset(const Com::Transform &transformA, const Com::HitBox &hitBoxA,
    const Com::Transform &transformB, const Com::HitBox &hitBoxB,
    sf::Vector2f offsetA, sf::Vector2f offsetB) {
    sf::Vector2f offsetA_scaled = offsetA * transformA.scale;
    sf::Vector2f offsetB_scaled = offsetB * transformB.scale;
    sf::Vector2f scaledHitBoxA = sf::Vector2f(hitBoxA.width * transformA.scale,
        hitBoxA.height * transformA.scale);
    sf::Vector2f scaledHitBoxB = sf::Vector2f(hitBoxB.width * transformB.scale,
        hitBoxB.height * transformB.scale);

    if (transformA.x + offsetA_scaled.x < transformB.x + scaledHitBoxB.x + offsetB_scaled.x &&
        transformA.x + scaledHitBoxA.x + offsetA_scaled.x > transformB.x + offsetB_scaled.x &&
        transformA.y + offsetA_scaled.y < transformB.y + scaledHitBoxB.y + offsetB_scaled.y &&
        transformA.y + scaledHitBoxA.y + offsetA_scaled.y > transformB.y + offsetB_scaled.y) {
        return true;
    }
    return false;
}

bool is_colliding(const Com::Transform &transformA, const Com::HitBox &hitBoxA,
    const Com::Transform &transformB, const Com::HitBox &hitBoxB, Rtype::Client::GameWorld &gameWorld) {
    sf::Vector2f offsetA = get_offset_from_transform(transformA, {hitBoxA.width, hitBoxA.height});
    sf::Vector2f offsetB = get_offset_from_transform(transformB, {hitBoxB.width, hitBoxB.height});

    offsetA.x = -offsetA.x;
    offsetA.y = -offsetA.y;
    offsetB.x = -offsetB.x;
    offsetB.y = -offsetB.y;

    return is_colliding_from_offset(transformA, hitBoxA, transformB, hitBoxB, offsetA, offsetB);
}

void compute_collision(Eng::sparse_array<Com::Solid> const &solids, int i, int j,
    Com::Transform &transformA, const Com::HitBox &hitBoxA,
    Com::Transform &transformB, const Com::HitBox &hitBoxB) {
    // If none are solid, nothing to resolve here
    bool aSolid = solids.has(i) ? solids[i]->isSolid : false;
    bool bSolid = solids.has(j) ? solids[j]->isSolid : false;
    if (!aSolid && !bSolid) return;

    // Check stuck/locked flags: if an entity is stuck, it must not be moved
    bool aStuck = solids.has(i) ? solids[i]->isLocked : false;
    bool bStuck = solids.has(j) ? solids[j]->isLocked : false;
    // If both are stuck, nothing to resolve
    if (aStuck && bStuck) return;

    // Compute offsets and scaled hitboxes (same as is_colliding)
    sf::Vector2f offsetA = get_offset_from_transform(transformA, {hitBoxA.width, hitBoxA.height});
    sf::Vector2f offsetB = get_offset_from_transform(transformB, {hitBoxB.width, hitBoxB.height});
    offsetA.x = -offsetA.x;
    offsetA.y = -offsetA.y;
    offsetB.x = -offsetB.x;
    offsetB.y = -offsetB.y;

    sf::Vector2f offsetA_scaled = offsetA * transformA.scale;
    sf::Vector2f offsetB_scaled = offsetB * transformB.scale;
    sf::Vector2f scaledA = sf::Vector2f(hitBoxA.width * transformA.scale,
        hitBoxA.height * transformA.scale);
    sf::Vector2f scaledB = sf::Vector2f(hitBoxB.width * transformB.scale,
        hitBoxB.height * transformB.scale);

    float a_min_x = transformA.x + offsetA_scaled.x;
    float a_max_x = a_min_x + scaledA.x;
    float a_min_y = transformA.y + offsetA_scaled.y;
    float a_max_y = a_min_y + scaledA.y;

    float b_min_x = transformB.x + offsetB_scaled.x;
    float b_max_x = b_min_x + scaledB.x;
    float b_min_y = transformB.y + offsetB_scaled.y;
    float b_max_y = b_min_y + scaledB.y;

    float overlapX = std::min(a_max_x, b_max_x) - std::max(a_min_x, b_min_x);
    float overlapY = std::min(a_max_y, b_max_y) - std::max(a_min_y, b_min_y);

    if (overlapX <= 0 || overlapY <= 0) return;

    // Separate along smallest penetration axis
    if (overlapX < overlapY) {
        float a_center = (a_min_x + a_max_x) / 2.0f;
        float b_center = (b_min_x + b_max_x) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool aCanMove = aSolid && !aStuck;
        bool bCanMove = bSolid && !bStuck;
        if (aCanMove && bCanMove) {
            transformA.x += direction * (overlapX / 2.0f);
            transformB.x -= direction * (overlapX / 2.0f);
        } else if (aCanMove && !bCanMove) {
            transformA.x += direction * overlapX;
        } else if (!aCanMove && bCanMove) {
            transformB.x -= direction * overlapX;
        }
    } else {
        float a_center = (a_min_y + a_max_y) / 2.0f;
        float b_center = (b_min_y + b_max_y) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool aCanMove = aSolid && !aStuck;
        bool bCanMove = bSolid && !bStuck;
        if (aCanMove && bCanMove) {
            transformA.y += direction * (overlapY / 2.0f);
            transformB.y -= direction * (overlapY / 2.0f);
        } else if (aCanMove && !bCanMove) {
            transformA.y += direction * overlapY;
        } else if (!aCanMove && bCanMove) {
            transformB.y -= direction * overlapY;
        }
    }
}

void collisionDetectionSystem(Eng::registry &reg, Rtype::Client::GameWorld &gameWorld,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids) {
    // Placeholder for collision detection logic
    for (auto &&[i, transformA, hitBoxA] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, transformB, hitBoxB] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j) continue;
            if (is_colliding(transformA, hitBoxA, transformB, hitBoxB, gameWorld)) {
                gameWorld.eventBus.publish(CollisionEvent{i, j, gameWorld});
                compute_collision(solids, i, j, transformA, hitBoxA, transformB, hitBoxB);
            }
        }
    }
}
}  // namespace Rtype::Client
