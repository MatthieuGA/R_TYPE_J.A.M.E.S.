#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void Server::RegisterSystems() {
    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Transform> &transforms,
            Engine::sparse_array<Component::Velocity> const &velocities) {
            for (auto &&[i, pos, vel] :
                make_indexed_zipper(transforms, velocities)) {
                auto &pos = transforms[i];
                auto &vel = velocities[i];
                pos->x += vel->vx * TICK_RATE_MS / 1000.0f;
                pos->y += vel->vy * TICK_RATE_MS / 1000.0f;
            }
        });

    std::cout << "Registered all systems" << std::endl;
}

}  // namespace server
