#include <iostream>
#include <utility>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

namespace server {

void Server::SetupEntitiesGame() {
    // Setup factory enemy info map
    FactoryActors::GetInstance().InitializeEnemyInfoMap("data/");

    for (const auto &pair : connection_manager_.GetClients()) {
        const auto &client = pair.second;
        if (client.player_id_ == 0)
            continue;  // skip unauthenticated

        auto entity = registry_.spawn_entity();
        FactoryActors::GetInstance().CreateActor(entity, registry_, "player",
            vector2f{100.f + client.player_id_ * 50.f, 300.f}, true);
    }

    // Spawns of enemies/projectiles
    for (int i = 0; i < 5; ++i) {
        auto enemy_entity = registry_.spawn_entity();
        FactoryActors::GetInstance().CreateActor(enemy_entity, registry_,
            "mermaid", vector2f{1400.f + i * 150.f, 200.f * (i + 1)}, false);
    }
}
}  // namespace server
