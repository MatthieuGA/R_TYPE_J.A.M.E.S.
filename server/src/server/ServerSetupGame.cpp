#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

namespace server {

/**
 * @brief Spawns an enemy at a random Y position on the right side of screen.
 *
 * @param reg The registry to spawn the enemy in.
 */
void SpawnEnemyFromRight(Engine::registry &reg) {
    auto enemy_entity = reg.spawn_entity();
    // Random Y position between 100 and 980 (leaving margin from edges)
    float random_y = 100.0f + static_cast<float>(std::rand() % 780);
    // Spawn just off-screen to the right (x = 2000 since screen is 1920 wide)

    std::vector<std::pair<std::string, int>> enemy_types = {{"health", 15},
        {"invinsibility", 15}, {"gatling", 15}, {"mermaid", 40},
        {"kami_fish", 40}, {"daemon", 15}};
    int tt = 0;
    for (const auto &et : enemy_types)
        tt += et.second;

    int r = std::rand() % tt;
    int v = 0;
    for (const auto &et : enemy_types) {
        v += et.second;
        if (r < v) {
            FactoryActors::GetInstance().CreateActor(enemy_entity, reg,
                et.first, vector2f{2000.f, random_y}, false);
            return;
        }
    }
}

void Server::SetupEntitiesGame() {
    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Setup factory enemy info map
    FactoryActors::GetInstance().InitializeEnemyInfoMap("data/");

    // Reset player trackingd q
    total_players_ = 0;
    alive_players_ = 0;

    for (const auto &pair : connection_manager_.GetClients()) {
        const auto &client = pair.second;
        if (client.player_id_ == 0)
            continue;  // skip unauthenticated

        auto entity = registry_.spawn_entity();
        FactoryActors::GetInstance().CreateActor(entity, registry_, "player",
            vector2f{100.f + client.player_id_ * 50.f, 300.f}, true);

        // Link player entity to player_id for disconnect cleanup
        auto &player_tag =
            registry_.GetComponent<Component::PlayerTag>(entity);
        player_tag.playerNumber = static_cast<int>(client.player_id_);

        // Override NetworkId to match player_id for input handling
        // (GAME_START sends player_id as controlled_entity_id)
        auto &network_id =
            registry_.GetComponent<Component::NetworkId>(entity);
        network_id.id = static_cast<uint32_t>(client.player_id_);

        // Track player count
        total_players_++;
        alive_players_++;
    }

    std::cout << "[SetupEntitiesGame] Spawned " << total_players_ << " players"
              << std::endl;

    // Create enemy spawner entity with timed event for infinite spawning
    auto spawner_entity = registry_.spawn_entity();
    registry_.AddComponent<Component::TimedEvents>(
        spawner_entity, Component::TimedEvents{});

    // Get reference and add spawning action (spawn every 2 seconds)
    auto &spawner_events =
        registry_.GetComponent<Component::TimedEvents>(spawner_entity);
    spawner_events.AddCooldownAction(
        [this](int /*entity_id*/) { SpawnEnemyFromRight(registry_); }, 2.0f);
}
}  // namespace server
