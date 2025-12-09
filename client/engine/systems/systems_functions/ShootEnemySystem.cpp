#include <string>
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

void UpdateCooldown(Com::EnemyShootTag &enemy_shoot, float deltaTime) {
    if (enemy_shoot.shoot_cooldown < enemy_shoot.shoot_cooldown_max) {
        enemy_shoot.shoot_cooldown += deltaTime;
    }
}

void HandleFrameBaseEvents(Eng::registry &reg, int entityId,
    Com::EnemyShootTag &enemy_shoot, Com::Transform const &transform,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites, int i) {
    auto &anim_sprite = animated_sprites[i];
    auto *current_anim = anim_sprite->GetCurrentAnimation();

    // Trigger attack animation when cooldown is ready
    if (enemy_shoot.shoot_cooldown > enemy_shoot.shoot_cooldown_max) {
        enemy_shoot.shoot_cooldown = 0.0f;
        anim_sprite->SetCurrentAnimation("Attack");
    }

    if (current_anim == nullptr)
        return;
    int current_frame = current_anim->currentFrame;
    std::string current_anim_name = anim_sprite->currentAnimation;

    // Check all frame events
    for (auto &frame_event : enemy_shoot.frame_events) {
        // Check if this event matches current animation and frame
        if (frame_event.animation_name == current_anim_name &&
            frame_event.trigger_frame == current_frame &&
            !frame_event.triggered) {
            // Execute event based on type
            if (frame_event.event_type ==
                Com::EnemyShootTag::FrameEvent::SHOOT_PROJECTILE) {
                sf::Vector2f shoot_direction =
                    GetShootDirection(transform, enemy_shoot.shoot_type);
                CreateEnemyProjectile(
                    reg, shoot_direction, enemy_shoot, i, transform);
            }
            frame_event.triggered = true;
        } else if (current_frame == 0) {
            // Reset triggered flag when animation resets
            frame_event.triggered = false;
        }
    }
}

void HandleCooldownBasedShooting(Eng::registry &reg, int entityId,
    Com::EnemyShootTag &enemy_shoot, Com::Transform const &transform,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites, int i) {
    if (enemy_shoot.shoot_cooldown > enemy_shoot.shoot_cooldown_max) {
        enemy_shoot.shoot_cooldown = 0.0f;

        sf::Vector2f shoot_direction =
            GetShootDirection(transform, enemy_shoot.shoot_type);

        CreateEnemyProjectile(reg, shoot_direction, enemy_shoot, i, transform);
        if (animated_sprites.has(i)) {
            auto &animSprite = animated_sprites[i];
            animSprite->SetCurrentAnimation("Attack");
        }
    }
}

void ShootEnemySystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::EnemyShootTag> &enemy_shoot_tags,
    Eng::sparse_array<Com::EnemyTag> const &enemy_tags) {
    for (auto &&[i, transform, enemy_shoot, enemy_tag] :
        make_indexed_zipper(transforms, enemy_shoot_tags, enemy_tags)) {
        UpdateCooldown(enemy_shoot, game_world.last_delta_);

        // Handle frame-based events if configured
        if (!enemy_shoot.frame_events.empty() && animated_sprites.has(i)) {
            HandleFrameBaseEvents(
                reg, i, enemy_shoot, transform, animated_sprites, i);
        } else {  // Fallback to cooldown-based shooting if no frame events
            HandleCooldownBasedShooting(
                reg, i, enemy_shoot, transform, animated_sprites, i);
        }
    }
}
}  // namespace Rtype::Client
