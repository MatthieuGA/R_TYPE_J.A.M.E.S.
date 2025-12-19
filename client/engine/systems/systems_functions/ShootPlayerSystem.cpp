#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/LayersConst.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

/**
 * @brief Creates a normal projectile entity.
 *
 * @param reg The registry to add the projectile to.
 * @param x The x position of the projectile.
 * @param y The y position of the projectile.
 * @param ownerId The ID of the entity that fired the projectile.
 */
void CreateProjectile(Eng::registry &reg, float x, float y, int ownerId,
    Engine::registry::entity_t projectile_entity) {
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable(
            "original_rtype/r-typesheet2.gif", LAYER_PROJECTILE));
    Component::AnimatedSprite animSprite(24, 32, 10);
    animSprite.AddAnimation("Death", "original_rtype/r-typesheet1.gif", 15, 15,
        2, 0.08f, false, sf::Vector2f(288.0f, 86.0f),
        sf::Vector2f(25.0f, 25.0f));
    reg.AddComponent<Component::AnimatedSprite>(
        projectile_entity, std::move(animSprite));
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{PLAYER_DAMAGE_PROJECTILE, {1.0f, 0},
            PLAYER_SPEED_PROJECTILE, ownerId, false});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{24.0f, 12.0f});
    reg.AddComponent<Component::ParticleEmitter>(projectile_entity,
        Component::ParticleEmitter(300, 500,
            Engine::Graphics::Color(0, 198, 255, 255),
            Engine::Graphics::Color(0, 198, 255, 0), sf::Vector2f(0.f, 2.f),
            true, 0.3f, 50.f, sf::Vector2f(-1.f, 0.f), 45.f, 0.f, 12.0f, 4.0f,
            1.5f, -1.0f, LAYER_PARTICLE));
}

/**
 * @brief Creates a charged projectile entity.
 *
 * @param reg The registry to add the projectile to.
 * @param x The x position of the projectile.
 * @param y The y position of the projectile.
 * @param ownerId The ID of the entity that fired the projectile.
 */
void createChargedProjectile(Eng::registry &reg, float x, float y, int ownerId,
    Engine::registry::entity_t projectile_entity) {
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable(
            "original_rtype/r-typesheet1.gif", LAYER_PROJECTILE));
    Component::AnimatedSprite animSprite(29, 22, 25);
    animSprite.AddAnimation("Death", "original_rtype/r-typesheet1.gif", 33, 31,
        6, 0.03f, false, sf::Vector2f(68.0f, 342.0f));
    reg.AddComponent<Component::AnimatedSprite>(
        projectile_entity, std::move(animSprite));
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{PLAYER_DAMAGE_CHARGED_PROJECTILE, {1.0f, 0.0f},
            PLAYER_SPEED_CHARGED_PROJECTILE, ownerId});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{29.0f, 22.0f});
    reg.AddComponent<Component::ParticleEmitter>(projectile_entity,
        Component::ParticleEmitter(300, 500,
            Engine::Graphics::Color(0, 198, 255, 255),
            Engine::Graphics::Color(0, 198, 255, 0), sf::Vector2f(0.f, 2.f),
            true, 0.4f, 75.f, sf::Vector2f(-1.f, 0.f), 55.f, 0.f, 20.0f, 6.0f,
            2.0f, -1.0f, LAYER_PARTICLE));
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
            CreateProjectile(
                reg, transform.x, transform.y, i, reg.SpawnEntity());
        }
        // Handle charged shooting
        if (input.shoot && input.last_shoot_state &&
            player_tag.charge_time < player_tag.charge_time_min)
            player_tag.charge_time += game_world.last_delta_;
        if (!input.shoot && input.last_shoot_state &&
            player_tag.charge_time >= player_tag.charge_time_min) {
            player_tag.charge_time = 0.0f;
            createChargedProjectile(
                reg, transform.x, transform.y, i, reg.SpawnEntity());
        }
        if (!input.shoot && input.last_shoot_state)
            player_tag.charge_time = 0.0f;
    }
}
}  // namespace Rtype::Client
