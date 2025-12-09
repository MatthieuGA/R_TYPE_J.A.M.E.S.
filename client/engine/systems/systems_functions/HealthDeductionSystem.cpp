
#include <SFML/Graphics.hpp>

#include "engine/CollidingTools.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief System to handle health deduction upon projectile collisions.
 *
 * This system checks for collisions between projectiles and entities
 * with Health components. It deducts health based on projectile damage
 * and removes projectiles upon impact. Entities with zero or less health
 * are removed from the registry.
 *
 * @param reg ECS registry used to access entities and components.
 * @param healths Sparse array of Health components.
 * @param enemyTags Sparse array of EnemyTag components.
 * @param playerTags Sparse array of PlayerTag components.
 * @param hitBoxes Sparse array of HitBox components.
 * @param transforms Sparse array of Transform components.
 * @param projectiles Sparse array of Projectile components.
 */
void HealthDeductionSystem(Eng::registry &reg,
    Eng::sparse_array<Component::Health> &healths,
    Eng::sparse_array<Component::EnemyTag> const &enemyTags,
    Eng::sparse_array<Component::PlayerTag> const &playerTags,
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
            if (projectile.isEnemyProjectile && enemyTags.has(i))
                continue;  // Enemy projectiles don't hit enemies
            if (!projectile.isEnemyProjectile && playerTags.has(i))
                continue;  // Player projectiles don't hit players

            // Simple AABB collision detection
            if (IsColliding(transform, hitBox, projTransform, projHitBox)) {
                // Collision detected, deduct health
                health.currentHealth -= projectile.damage;
                if (animated_sprites.has(i)) {
                    auto &animSprite = animated_sprites[i];
                    animSprite->SetCurrentAnimation("Hit", true);
                    animSprite->GetCurrentAnimation()->currentFrame = 1;
                }

                // Remove projectile after hit
                reg.KillEntity(projEntity);
            }
        }

        // Handle entity death
        if (health.currentHealth <= 0)
            reg.KillEntity(entity);
    }
}

}  // namespace Rtype::Client
