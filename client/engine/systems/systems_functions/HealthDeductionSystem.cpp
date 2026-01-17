
#include <vector>

#include <SFML/Graphics.hpp>

#include "engine/CollidingTools.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Information about a collision to be processed.
 */
struct CollisionInfo {
    std::size_t entity_index;
    Engine::entity proj_entity;
    std::size_t proj_index;
    bool has_anim_sprite;
};

/**
 * @brief Process a collision between an entity and a projectile.
 *
 * Handles audio, health bar, and hit animation for the entity.
 * Handles projectile removal or death animation.
 *
 * @param reg The registry.
 * @param game_world The game world (for audio access).
 * @param collision The collision info to process.
 */
void ProcessCollision(Eng::registry &reg, GameWorld &game_world,
    const CollisionInfo &collision) {
    auto &health_bars = reg.GetComponents<Component::HealthBar>();
    auto &animated_sprites = reg.GetComponents<Component::AnimatedSprite>();
    auto &healths = reg.GetComponents<Component::Health>();

    std::size_t i = collision.entity_index;
    Engine::entity proj_entity = collision.proj_entity;
    std::size_t j = collision.proj_index;
    // Check if this is a player getting hit and play damage sound
    if (reg.GetComponents<Component::PlayerTag>().has(i)) {
        if (game_world.audio_manager_) {
            game_world.audio_manager_->PlaySound("player_damage", 0.1f);
        }
    }

    if (health_bars.has(i)) {
        auto &bar = health_bars[i];
        bar->timer_damage = 0.0f;
    }

    if (animated_sprites.has(i) && healths.has(i)) {
        auto &health = healths[i];
        auto &anim_sprite = animated_sprites[i];
        if (health->invincibilityDuration <= 0.0f) {
            anim_sprite->SetCurrentAnimation("Hit", true);
            anim_sprite->GetCurrentAnimation()->current_frame = 1;
        }
    }

    // Remove the projectile component after collision
    reg.RemoveComponent<Component::Projectile>(proj_entity);

    if (collision.has_anim_sprite && animated_sprites.has(j)) {
        auto &proj_anim_sprite = animated_sprites[j];
        proj_anim_sprite->SetCurrentAnimation("Death", false);
        proj_anim_sprite->animated = true;
        reg.AddComponent<Component::AnimationDeath>(
            proj_entity, Component::AnimationDeath{true});
    } else {
        reg.KillEntity(proj_entity);
    }
}

/**
 * @brief System to handle health deduction upon projectile collisions.
 *
 * This system checks for collisions between projectiles and entities
 * with Health components. It deducts health based on projectile damage
 * and removes projectiles upon impact. Entities with zero or less health
 * are removed from the registry.
 *
 * @param reg ECS registry used to access entities and components.
 * @param game_world The game world (for audio access).
 * @param healths Sparse array of Health components.
 * @param enemyTags Sparse array of EnemyTag components.
 * @param playerTags Sparse array of PlayerTag components.
 * @param hitBoxes Sparse array of HitBox components.
 * @param transforms Sparse array of Transform components.
 * @param projectiles Sparse array of Projectile components.
 */
void HealthDeductionSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Component::Health> &healths,
    Eng::sparse_array<Component::HealthBar> &health_bars,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::Drawable> &drawables,
    Eng::sparse_array<Component::HitBox> const &hitBoxes,
    Eng::sparse_array<Component::Transform> const &transforms,
    Eng::sparse_array<Component::Projectile> const &projectiles) {
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!transform.parent_entity.has_value())
            continue;  // Skip if not a child entity
        auto parentEntity = transform.parent_entity.value();
        try {
            if (!reg.GetComponents<Component::PlayerTag>().has(parentEntity))
                continue;  // Skip if parent is not a player
            auto &health = reg.GetComponent<Component::Health>(
                reg.EntityFromIndex(parentEntity));
            if (health.invincibilityDuration > 0.0f)
                drawable.opacity = 1.f;
            else
                drawable.opacity = 0.0f;
        } catch (...) {
            continue;  // Parent entity might not have PlayerTag or Health
        }
    }
    // Collect all collisions first to avoid modifying sparse arrays during
    // iteration
    std::vector<CollisionInfo> collisions;

    for (auto &&[i, health, hitBox, transform] :
        make_indexed_zipper(healths, hitBoxes, transforms)) {
        // Check for collisions with projectiles
        for (auto &&[j, projectile, projHitBox, projTransform] :
            make_indexed_zipper(projectiles, hitBoxes, transforms)) {
            try {
                if (reg.GetComponents<Component::PowerUp>().has(i))
                    continue;  // Player projectiles don't hit players
                if (projectile.isEnemyProjectile &&
                    reg.GetComponents<Component::EnemyTag>().has(i))
                    continue;  // Enemy projectiles don't hit enemies
                if (!projectile.isEnemyProjectile &&
                    reg.GetComponents<Component::PlayerTag>().has(i))
                    continue;  // Player projectiles don't hit players
            } catch (...) {}

            // Simple AABB collision detection
            if (IsColliding(transform, hitBox, projTransform, projHitBox)) {
                // Record collision for later processing
                Engine::entity proj_entity = reg.EntityFromIndex(j);
                collisions.push_back(
                    CollisionInfo{i, proj_entity, j, animated_sprites.has(j)});
            }
        }
    }

    // Now process all collisions after iteration is complete
    for (const auto &collision : collisions) {
        ProcessCollision(reg, game_world, collision);
    }
}

}  // namespace Rtype::Client
