
#include <SFML/Graphics.hpp>

#include "engine/CollidingTools.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

void DeathHandling(Eng::registry &reg,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::entity entity, std::size_t i) {
    // Mark entity with AnimationDeath component to trigger death anim
    reg.AddComponent<Component::AnimationDeath>(
        entity, Component::AnimationDeath{true});
    reg.RemoveComponent<Component::Health>(entity);
    reg.RemoveComponent<Component::HitBox>(entity);
    if (reg.GetComponents<Component::PlayerTag>().has(i))
        reg.RemoveComponent<Component::PlayerTag>(entity);
    if (reg.GetComponents<Component::EnemyTag>().has(i))
        reg.RemoveComponent<Component::EnemyTag>(entity);
    reg.RemoveComponent<Component::TimedEvents>(entity);
    reg.RemoveComponent<Component::FrameEvents>(entity);

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

void HandleCollision(Eng::registry &reg, Component::Health &health,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
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
                HandleCollision(reg, health, animated_sprites, i, projEntity,
                    j, projectile);
            }
        }

        if (health.currentHealth <= 0)
            DeathHandling(reg, animated_sprites, entity, i);
    }
}

}  // namespace Rtype::Client
