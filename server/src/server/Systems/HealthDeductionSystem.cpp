#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/CollidingTools.hpp"
#include "server/systems/OriginTool.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {
/**
 * @brief Handle the death of an entity.
 *
 * Marks the entity for death animation and removes relevant components.
 *
 * @param reg The registry.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param entity The entity to handle death for.
 * @param i The index of the entity in the registry.
 */
void DeathHandling(Engine::registry &reg,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::entity entity, std::size_t i) {
    // Check if this is a player dying - notify server before removing tag
    if (reg.GetComponents<Component::PlayerTag>().has(i)) {
        std::cout << "[DeathHandling] Player at index " << i << " died!"
                  << std::endl;
        if (Server::GetInstance()) {
            Server::GetInstance()->NotifyPlayerDeath();
        }
    }

    // Mark entity with AnimationDeath component to trigger death anim
    reg.AddComponent<Component::AnimationDeath>(
        entity, Component::AnimationDeath{true});
    reg.RemoveComponent<Component::Health>(entity);
    reg.RemoveComponent<Component::HitBox>(entity);
    // Remove PlayerTag since we've already notified the server
    if (reg.GetComponents<Component::PlayerTag>().has(i))
        reg.RemoveComponent<Component::PlayerTag>(entity);
    if (reg.GetComponents<Component::EnemyTag>().has(i))
        reg.RemoveComponent<Component::EnemyTag>(entity);
    reg.RemoveComponent<Component::TimedEvents>(entity);
    reg.RemoveComponent<Component::FrameEvents>(entity);
    reg.RemoveComponent<Component::PatternMovement>(entity);

    // Play death animation
    if (animated_sprites.has(i)) {
        auto &animSprite = animated_sprites[i];
        animSprite->SetCurrentAnimation("Death", true);
        animSprite->animated = true;
    } else {
        // No animated sprite, remove entity immediately
        reg.KillEntity(entity);
    }
}

/**
 * @brief Handle collision between an entity and a projectile.
 *
 * Deducts health, updates health bar, triggers hit animation,
 * and handles projectile removal.
 *
 * @param reg The registry.
 * @param health The Health component of the entity.
 * @param health_bars Sparse array of HealthBar components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param i Index of the entity in the registry.
 * @param projEntity The entity ID of the projectile.
 * @param j Index of the projectile in the registry.
 * @param projectile The Projectile component of the projectile.
 */
void HandleCollision(Engine::registry &reg, Component::Health &health,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    std::size_t i, Engine::entity projEntity, std::size_t j,
    const Component::Projectile &projectile) {
    health.currentHealth -= projectile.damage;

    if (animated_sprites.has(i)) {
        auto &animSprite = animated_sprites[i];
        animSprite->SetCurrentAnimation("Hit", true);
        animSprite->GetCurrentAnimation()->current_frame = 1;
    }

    reg.RemoveComponent<Component::Projectile>(projEntity);
    if (animated_sprites.has(j)) {
        auto &projAnimSprite = animated_sprites[j];
        projAnimSprite->SetCurrentAnimation("Death", false);
        projAnimSprite->animated = true;
        reg.AddComponent<Component::AnimationDeath>(
            projEntity, Component::AnimationDeath{true});
    } else {
        reg.KillEntity(projEntity);
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
 * @param healths Sparse array of Health components.
 * @param enemyTags Sparse array of EnemyTag components.
 * @param playerTags Sparse array of PlayerTag components.
 * @param hitBoxes Sparse array of HitBox components.
 * @param transforms Sparse array of Transform components.
 * @param projectiles Sparse array of Projectile components.
 */
void HealthDeductionSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Health> &healths,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::sparse_array<Component::HitBox> const &hitBoxes,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::Projectile> const &projectiles) {
    for (auto &&[i, health, hitBox, transform] :
        make_indexed_zipper(healths, hitBoxes, transforms)) {
        Engine::entity entity = reg.EntityFromIndex(i);

        // Check for collisions with projectiles
        for (auto &&[j, projectile, projHitBox, projTransform] :
            make_indexed_zipper(projectiles, hitBoxes, transforms)) {
            Engine::entity projEntity = reg.EntityFromIndex(j);

            // Skip self-collision
            if (i == j)
                continue;
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
                HandleCollision(reg, health, animated_sprites, i, projEntity,
                    j, projectile);
            }
        }

        if (health.currentHealth <= 0)
            DeathHandling(reg, animated_sprites, entity, i);
    }

    // Check if killable enemy projectiles setting is enabled
    extern bool g_killable_enemy_projectiles;
    if (g_killable_enemy_projectiles) {
        // Check for projectile-to-projectile collisions
        // Player projectiles can destroy enemy projectiles
        for (auto &&[i, projectile1, hitBox1, transform1] :
            make_indexed_zipper(projectiles, hitBoxes, transforms)) {
            // Skip if not a player projectile
            if (projectile1.isEnemyProjectile)
                continue;

            Engine::entity playerProj = reg.EntityFromIndex(i);

            for (auto &&[j, projectile2, hitBox2, transform2] :
                make_indexed_zipper(projectiles, hitBoxes, transforms)) {
                // Skip self-collision and non-enemy projectiles
                if (i == j || !projectile2.isEnemyProjectile)
                    continue;

                Engine::entity enemyProj = reg.EntityFromIndex(j);

                // Check collision between player projectile and enemy
                // projectile
                if (IsColliding(transform1, hitBox1, transform2, hitBox2)) {
                    // Destroy both projectiles
                    reg.RemoveComponent<Component::Projectile>(playerProj);
                    reg.RemoveComponent<Component::Projectile>(enemyProj);

                    // Trigger death animations if available
                    if (animated_sprites.has(i)) {
                        auto &animSprite1 = animated_sprites[i];
                        animSprite1->SetCurrentAnimation("Death", false);
                        animSprite1->animated = true;
                        reg.AddComponent<Component::AnimationDeath>(
                            playerProj, Component::AnimationDeath{true});
                    } else {
                        reg.KillEntity(playerProj);
                    }

                    if (animated_sprites.has(j)) {
                        auto &animSprite2 = animated_sprites[j];
                        animSprite2->SetCurrentAnimation("Death", false);
                        animSprite2->animated = true;
                        reg.AddComponent<Component::AnimationDeath>(
                            enemyProj, Component::AnimationDeath{true});
                    } else {
                        reg.KillEntity(enemyProj);
                    }

                    // Break inner loop since player projectile is destroyed
                    break;
                }
            }
        }
    }
}

}  // namespace server
