#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void CreateEnemyProjectile(Eng::registry &reg, sf::Vector2f direction,
    Com::EnemyShootTag &enemy_shoot, int ownerId,
    Com::Transform const &transform) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{
            transform.x + (enemy_shoot.offset_shoot_position.x *
                              std::abs(transform.scale.x)),
            transform.y + (enemy_shoot.offset_shoot_position.y *
                              std::abs(transform.scale.y)),
            0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable("ennemies/4/Projectile.png", -1));
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{6.0f, 6.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

sf::Vector2f GetShootDirection(Component::Transform const &enemy_transform,
    Component::EnemyShootTag::ShootType shoot_type) {
    return sf::Vector2f(-1.0f, 0.0f);
}

void ShootEnemySystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::EnemyShootTag> &enemy_shoot_tags,
    Eng::sparse_array<Com::EnemyTag> const &enemy_tags) {
    for (auto &&[i, transform, enemy_shoot, enemy_tag] :
        make_indexed_zipper(transforms, enemy_shoot_tags, enemy_tags)) {
        // Handle shooting normal projectiles
        if (enemy_shoot.shoot_cooldown <= enemy_shoot.shoot_cooldown_max) {
            enemy_shoot.shoot_cooldown += game_world.last_delta_;
        } else {
            enemy_shoot.shoot_cooldown = 0.0f;

            sf::Vector2f shoot_direction =
                GetShootDirection(transform, enemy_shoot.shoot_type);

            CreateEnemyProjectile(
                reg, shoot_direction, enemy_shoot, i, transform);
            if (animated_sprites.has(i)) {
                auto &animSprite = animated_sprites[i];
                animSprite->SetCurrentAnimation("Attack");
            }
        }
    }
}
}  // namespace Rtype::Client
