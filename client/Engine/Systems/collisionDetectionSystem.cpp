#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/Events/EngineEvent.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

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
    sf::Vector2f offsetA;
    sf::Vector2f offsetB;

    if (transformA.origin == Com::Transform::CENTER) {
        offsetA.x = -hitBoxA.width / 2.0f;
        offsetA.y = -hitBoxA.height / 2.0f;
    }
    if (transformB.origin == Com::Transform::CENTER) {
        offsetB.x = -hitBoxB.width / 2.0f;
        offsetB.y = -hitBoxB.height / 2.0f;
    }
    return is_colliding_from_offset(transformA, hitBoxA, transformB, hitBoxB, offsetA, offsetB);
}

void collisionDetectionSystem(Eng::registry &reg, Rtype::Client::GameWorld &gameWorld,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes) {
    // Placeholder for collision detection logic
    for (auto &&[i, transformA, hitBoxA] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, transformB, hitBoxB] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j) continue;  // Avoid duplicate checks and self-collision
            // Simple AABB collision detection
            if (is_colliding(transformA, hitBoxA, transformB, hitBoxB, gameWorld))
                gameWorld.eventBus.publish(CollisionEvent(i, j, gameWorld));
        }
    }
}
}  // namespace Rtype::Client
