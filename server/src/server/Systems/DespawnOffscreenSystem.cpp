/**
 * @file DespawnOffscreenSystem.cpp
 * @brief System that despawns entities that have moved off the left screen
 * edge.
 */

#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

/// Threshold x position below which entities are despawned (off left screen).
static constexpr float kDespawnThresholdX = -150.0f;

void DespawnOffscreenSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::PlayerTag> const &player_tags) {
    // Collect entities to despawn (can't modify while iterating)
    std::vector<std::size_t> entities_to_despawn;

    for (auto &&[i, transform] : make_indexed_zipper(transforms)) {
        // Skip players - they should never be despawned by this system
        if (player_tags.size() > i && player_tags[i].has_value()) {
            continue;
        }

        // Check if entity has moved past the left edge
        if (transform.x < kDespawnThresholdX) {
            entities_to_despawn.push_back(i);
        }
    }

    // Despawn collected entities
    for (auto entity_idx : entities_to_despawn) {
        auto entity = reg.entity_from_index(entity_idx);
        reg.kill_entity(entity);
    }
}

}  // namespace server
