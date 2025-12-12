#include <iostream>
#include <utility>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"

namespace server {

void Server::SetupEntityiesGame() {
    // Example: Create player entities
    for (uint8_t i = 1; i <= connection_manager_.GetClients().size(); ++i) {
        auto entity = registry_.spawn_entity();
        registry_.add_component(
            entity, Component::Transform{50.0f, 400.0f, 0.0f, {1.0f, 1.0f}});
        registry_.add_component(entity, Component::Velocity{0.0f, 0.0f});
        std::cout << "Created entity for Player" << static_cast<int>(i)
                  << std::endl;
    }
}
}  // namespace server
