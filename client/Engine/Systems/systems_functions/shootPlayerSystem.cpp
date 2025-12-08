#include <SFML/Graphics.hpp>

#include "engine/systems/initRegistrySystems.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void createProjectile(Eng::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable("OriginalRtype/r-typesheet2.gif", -1));
    reg.AddComponent<Component::AnimatedSprite>(
        projectile_entity, Component::AnimatedSprite{24, 32, 10});
    reg.AddComponent<Component::Projectile>(
        projectile_entity, Component::Projectile{PLAYER_DAMAGE_PROJECTILE,
                               PLAYER_SPEED_PROJECTILE, ownerId});
}

void createChargedProjectile(
    Eng::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable("OriginalRtype/r-typesheet1.gif", -1));
    reg.AddComponent<Component::AnimatedSprite>(
        projectile_entity, Component::AnimatedSprite{29, 22, 25});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{PLAYER_DAMAGE_CHARGED_PROJECTILE,
            PLAYER_SPEED_CHARGED_PROJECTILE, ownerId});
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
void ShootPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::PlayerTag> &player_tags) {
    for (auto &&[i, transform, input, player_tag] :
        make_indexed_zipper(transforms, inputs, player_tags)) {
        // Handle shooting normal projectiles
        if (player_tag.shoot_cooldown > 0.0f)
            player_tag.shoot_cooldown -= game_world.last_delta_;
        if (input.shoot && !input.last_shoot_state &&
            player_tag.shoot_cooldown <= 0.0f) {
            player_tag.charge_time = 0.0f;
            player_tag.shoot_cooldown = player_tag.shoot_cooldown_max;
            createProjectile(reg, transform.x, transform.y, i);
        }
        // Handle charged shooting
        if (input.shoot && input.last_shoot_state &&
            player_tag.charge_time < player_tag.charge_time_min)
            player_tag.charge_time += game_world.last_delta_;
        if (!input.shoot && input.last_shoot_state &&
            player_tag.charge_time >= player_tag.charge_time_min) {
            player_tag.charge_time = 0.0f;
            createChargedProjectile(reg, transform.x, transform.y, i);
        }
        if (!input.shoot && input.last_shoot_state)
            player_tag.charge_time = 0.0f;
    }
}
}  // namespace Rtype::Client
