#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void collisionDetectionSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes) {
    // Placeholder for collision detection logic
    for (auto &&[i, transformA, hitBoxA] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, transformB, hitBoxB] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j) continue;  // Avoid duplicate checks and self-collision
            // Simple AABB collision detection
            if (transformA.x < transformB.x + hitBoxB.width &&
                transformA.x + hitBoxA.width > transformB.x &&
                transformA.y < transformB.y + hitBoxB.height &&
                transformA.y + hitBoxA.height > transformB.y) {
                std::cout << "Collision detected between entities "
                          << i << " and " << j << std::endl;
            }
        }
    }
}
}  // namespace Rtype::Client
