#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

/**
 * @brief Creates a normal projectile entity.
 *
 * @param reg The registry to add the projectile to.
 * @param x The x position of the projectile.
 * @param y The y position of the projectile.
 * @param ownerId The ID of the entity that fired the projectile.
 */
void createProjectileGatling(
    Engine::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Component::Transform::CENTER});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{Component::Projectile::ProjectileType::Gatling,
            3, {1.0f, 0}, 1500.0f, ownerId, false});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{24.0f, 12.0f});
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
}

/**
 * @brief Update player animated sprite frame selection based on vertical
 * velocity.
 *
 * Chooses a frame index representing up/down/neutral movement states.
 *
 * @param reg Engine registry (unused)
 * @param player_tags Sparse array of PlayerTag components
 * @param velocities Sparse array of Velocity components
 * @param animated_sprites Sparse array of AnimatedSprite components to update
 */
void PlayerGatlingSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::PlayerTag> &player_tags) {
    for (auto &&[i, player_tag, transforms] :
        make_indexed_zipper(player_tags, transforms)) {
        if (player_tag.gatling_duration > 0.0f) {
            player_tag.gatling_duration -= g_frame_delta_seconds;
            if (player_tag.gatling_duration < 0.0f)
                player_tag.gatling_duration = 0.0f;
        }

        if (player_tag.gatling_duration > 0.0f) {
            player_tag.clock_shoot_gatling -= g_frame_delta_seconds;
            if (player_tag.clock_shoot_gatling <= 0.0f) {
                createProjectileGatling(reg, transforms.x + 50.0f,
                    transforms.y + 10.f, static_cast<int>(i));
                createProjectileGatling(reg, transforms.x + 50.0f,
                    transforms.y - 10.f, static_cast<int>(i));
                player_tag.clock_shoot_gatling =
                    player_tag.delta_shoot_gatling;
            }
        }
    }
}
}  // namespace server
