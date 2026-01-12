#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

const constexpr float PLAYER_SHOOT_COOLDOWN_MS = 250.0f;
const constexpr float PLAYER_CHARGED_SHOOT_COOLDOWN_MS = 1000.0f;
const constexpr int PLAYER_DAMAGE_PROJECTILE = 10;
const constexpr int PLAYER_DAMAGE_CHARGED_PROJECTILE = 30;
const constexpr float PLAYER_SPEED_PROJECTILE = 800.0f;
const constexpr float PLAYER_SPEED_CHARGED_PROJECTILE = 600.0f;
const constexpr float PLAYER_CHARGED_SHOOT_HOLD_TIME_MS = 1500.0f;
const constexpr float PLAYER_MAX_CHARGE_TIME_MS = 3000.0f;
const constexpr float PLAYER_MIN_CHARGE_TIME_MS = 500.0f;

/**
 * @brief Creates a normal projectile entity.
 *
 * @param reg The registry to add the projectile to.
 * @param x The x position of the projectile.
 * @param y The y position of the projectile.
 * @param ownerId The ID of the entity that fired the projectile.
 */
void createProjectile(Engine::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Component::Transform::CENTER});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{Component::Projectile::ProjectileType::Normal,
            PLAYER_DAMAGE_PROJECTILE, {1.0f, 0}, PLAYER_SPEED_PROJECTILE,
            ownerId, false});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{24.0f, 12.0f});
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
}

/**
 * @brief Creates a charged projectile entity.
 *
 * @param reg The registry to add the projectile to.
 * @param x The x position of the projectile.
 * @param y The y position of the projectile.
 * @param ownerId The ID of the entity that fired the projectile.
 */
void createChargedProjectile(
    Engine::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Component::Transform::CENTER});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{Component::Projectile::ProjectileType::Charged,
            PLAYER_DAMAGE_CHARGED_PROJECTILE, {1.0f, 0.0f},
            PLAYER_SPEED_CHARGED_PROJECTILE, ownerId});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{29.0f, 22.0f});
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
}

/**
 * @brief System to handle player shooting mechanics.
 *
 * This system processes player inputs to manage shooting normal and charged
 * projectiles.
 *
 * @param reg The registry containing components.
 * @param game_world The game world for accessing global state.
 * @param transforms Sparse array of Transform components.
 * @param inputs Sparse array of Inputs components.
 * @param player_tags Sparse array of PlayerTag components.
 */
void ShootPlayerSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Inputs> &inputs,
    Engine::sparse_array<Component::PlayerTag> &player_tags) {
    for (auto &&[i, transform, input, player_tag] :
        make_indexed_zipper(transforms, inputs, player_tags)) {
        // Handle shooting normal projectiles
        if (player_tag.shoot_cooldown > 0.0f)
            player_tag.shoot_cooldown -= g_frame_delta_seconds;
        if (input.shoot && !input.last_shoot_state &&
            player_tag.shoot_cooldown <= 0.0f) {
            player_tag.charge_time = 0.0f;
            player_tag.shoot_cooldown = player_tag.shoot_cooldown_max;
            createProjectile(reg, transform.x, transform.y, i);
        }
        // Handle charged shooting
        if (input.shoot && input.last_shoot_state &&
            player_tag.charge_time < player_tag.charge_time_min)
            player_tag.charge_time += g_frame_delta_seconds;
        if (!input.shoot && input.last_shoot_state &&
            player_tag.charge_time >= player_tag.charge_time_min) {
            player_tag.charge_time = 0.0f;
            createChargedProjectile(reg, transform.x, transform.y, i);
        }
        if (!input.shoot && input.last_shoot_state)
            player_tag.charge_time = 0.0f;
        input.last_shoot_state = input.shoot;
    }
}

}  // namespace server
