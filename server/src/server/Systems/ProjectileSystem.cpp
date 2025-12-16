#include <cmath>
#include <iostream>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameWorldDatas.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void ProjectileSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Projectile> &projectiles) {
    std::vector<Engine::registry::entity_t> to_kill;

    for (auto &&[i, transform, projectile] :
        make_indexed_zipper(transforms, projectiles)) {
        // normalize direction
        float length =
            std::sqrt(projectile.direction.x * projectile.direction.x +
                      projectile.direction.y * projectile.direction.y);
        if (length != 0.0f) {
            projectile.direction.x /= length;
            projectile.direction.y /= length;
        }

        // Convert TICK_RATE_MS from milliseconds to seconds for proper
        // velocity calculation
        transform.x +=
            projectile.speed * TICK_RATE_SECONDS * projectile.direction.x;
        transform.y +=
            projectile.speed * TICK_RATE_SECONDS * projectile.direction.y;

        if (transform.x > WINDOW_WIDTH + 100.f || transform.x < -100.f ||
            transform.y > WINDOW_HEIGHT + 100.f || transform.y < -100.f) {
            to_kill.push_back(reg.EntityFromIndex(i));
        }
    }

    for (auto const &e : to_kill) {
        reg.KillEntity(e);
    }
}

}  // namespace server
