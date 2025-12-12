#include <iostream>
#include <utility>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

void Server::SetupEntityiesGame() {
    // Create one player entity per authenticated client, using the
    // client's assigned player_id as the NetworkId so clients can map
    // controlled entity easily.
    for (const auto &pair : connection_manager_.GetClients()) {
        const auto &client = pair.second;
        if (client.player_id_ == 0)
            continue;  // skip unauthenticated

        auto entity = registry_.spawn_entity();
        // Place players at distinct Y positions based on player_id
        float y = 200.0f * (static_cast<int>(client.player_id_) + 1);
        registry_.add_component(
            entity, Component::Transform{50.0f, y, 0.0f, {1.0f, 1.0f}});
        registry_.add_component(entity, Component::Velocity{100.0f, 0.0f});
        registry_.add_component(
            entity, Component::NetworkId{client.player_id_});
        std::cout << "Created entity for Player "
                  << static_cast<int>(client.player_id_) << std::endl;
    }
}
}  // namespace server
