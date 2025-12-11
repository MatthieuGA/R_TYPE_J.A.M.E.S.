#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"

namespace server {

void Server::RegisterSystems() {
    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Transform> &transforms,
            Engine::sparse_array<Component::Velocity> &velocities) {
            for (size_t i = 0; i < transforms.size() && i < velocities.size();
                ++i) {
                if (transforms.has(i) && velocities.has(i)) {
                    auto &pos = transforms[i];
                    auto &vel = velocities[i];
                    pos->x += vel->vx;
                    pos->y += vel->vy;
                }
            }
        });

    std::cout << "Registered all systems" << std::endl;
}

}  // namespace server
