
#include <SFML/Graphics.hpp>

#include "engine/CollidingTools.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Handle collision between an entity and a projectile.
 *
 * Deducts health, updates health bar, triggers hit animation,
 * and handles projectile removal.
 *
 * @param reg The registry.
 * @param game_world The game world (for audio access).
 * @param health The Health component of the entity.
 * @param health_bars Sparse array of HealthBar components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param i Index of the entity in the registry.
 * @param projEntity The entity ID of the projectile.
 * @param j Index of the projectile in the registry.
 * @param projectile The Projectile component of the projectile.
 */
void HandleCollision(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Component::HealthBar> &health_bars,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    std::size_t i, Engine::entity projEntity, std::size_t j,
    const Component::Projectile &projectile) {
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

    if (animated_sprites.has(i)) {
        auto &animSprite = animated_sprites[i];
        animSprite->SetCurrentAnimation("Hit", true);
        animSprite->GetCurrentAnimation()->current_frame = 1;
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
    Eng::sparse_array<Component::HitBox> const &hitBoxes,
    Eng::sparse_array<Component::Transform> const &transforms,
    Eng::sparse_array<Component::Projectile> const &projectiles) {
    for (auto &&[i, health, hitBox, transform] :
        make_indexed_zipper(healths, hitBoxes, transforms)) {
        Engine::entity entity = reg.EntityFromIndex(i);

        // Check for collisions with projectiles
        for (auto &&[j, projectile, projHitBox, projTransform] :
            make_indexed_zipper(projectiles, hitBoxes, transforms)) {
            Engine::entity projEntity = reg.EntityFromIndex(j);
            try {
                if (projectile.isEnemyProjectile &&
                    reg.GetComponents<Component::EnemyTag>().has(i))
                    continue;  // Enemy projectiles don't hit enemies
                if (!projectile.isEnemyProjectile &&
                    reg.GetComponents<Component::PlayerTag>().has(i))
                    continue;  // Player projectiles don't hit players
            } catch (...) {}

            // Simple AABB collision detection
            if (IsColliding(transform, hitBox, projTransform, projHitBox)) {
                // Collision detected, deduct health
                HandleCollision(reg, game_world, health_bars, animated_sprites,
                    i, projEntity, j, projectile);
            }
        }
    }
}

}  // namespace Rtype::Client
