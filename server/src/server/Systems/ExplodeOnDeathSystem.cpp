#include <cmath>
#include <iostream>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/CollidingTools.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

static inline float DistanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief When an entity with ExplodeOnDeath dies, damage nearby entities
 * and trigger its "Attack" animation (if present).
 */
void ExplodeOnDeathSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Health> &healths,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::sparse_array<Component::ExplodeOnDeath> &explode_on_deaths,
    Engine::sparse_array<Component::AnimationDeath> &animation_deaths,
    Engine::sparse_array<Component::HitBox> const &hitBoxes,
    Engine::sparse_array<Component::PlayerTag> const &player_tags) {
    // Precompute list of entities with health and their positions
    struct TargetInfo {
        std::size_t id;
        float x;
        float y;
    };

    std::vector<TargetInfo> targets;
    targets.reserve(healths.size());
    for (auto &&[tid, h] : make_indexed_zipper(healths)) {
        if (transforms.has(tid)) {
            auto &t = transforms[tid];
            targets.push_back({tid, t->x, t->y});
        }
    }

    // For each entity that can explode, check death and apply AoE damage once
    for (auto &&[eid, expl] : make_indexed_zipper(explode_on_deaths)) {
        // Skip if already processed or if no transform present
        if (expl.exploded)
            continue;
        if (!transforms.has(eid))
            continue;

        // Determine if entity is dead: either health <= 0 or flagged
        // AnimationDeath. Also trigger explosion when colliding with a player.
        bool is_dead = false;
        // Check melee contact with any player
        bool melee_triggered = false;
        for (auto &&[pid, ptag] : make_indexed_zipper(player_tags)) {
            if (!transforms.has(pid) || !hitBoxes.has(pid))
                continue;
            if (!hitBoxes.has(eid))
                continue;
            auto t_a = transforms[eid];
            auto t_b = transforms[pid];
            if (IsColliding(*t_a, *hitBoxes[eid], *t_b, *hitBoxes[pid])) {
                is_dead = true;
                melee_triggered = true;
                break;
            }
        }
        if (healths.has(eid)) {
            auto &h = healths[eid];
            if (h->currentHealth <= 0)
                is_dead = true;
        }
        if (!is_dead && animation_deaths.has(eid))
            is_dead = true;
        if (!is_dead)
            continue;

        // Mark as exploded to avoid re-triggering
        expl.exploded = true;

        auto &t_self = transforms[eid];
        float r2 = expl.radius * expl.radius;

        // Apply damage to all targets within radius (exclude self)
        for (auto &ti : targets) {
            if (ti.id == eid)
                continue;
            float d2 = DistanceSquared(t_self->x, t_self->y, ti.x, ti.y);
            if (d2 <= r2) {
                if (healths.has(ti.id)) {
                    auto &target_health = healths[ti.id];
                    target_health->currentHealth -= expl.damage;
                    // Trigger Hit animation on damaged target if present
                    if (animated_sprites.has(ti.id)) {
                        auto &animSprite = animated_sprites[ti.id];
                        animSprite->SetCurrentAnimation("Hit", true);
                        if (animSprite->GetCurrentAnimation())
                            animSprite->GetCurrentAnimation()->current_frame =
                                1;
                    }
                    // Clamp
                    if (target_health->currentHealth < 0)
                        target_health->currentHealth = 0;
                }
            }
        }

        // Trigger Attack animation on the exploding entity if present
        if (animated_sprites.has(eid)) {
            auto &anim = animated_sprites[eid];
            anim->SetCurrentAnimation("Attack", true, false);
        }

        // If explosion was triggered by melee (not by HealthDeduction),
        // schedule the entity for removal similar to a death: remove
        // interaction components and add AnimationDeath so
        // DeathAnimationSystem will clean it up after the Attack animation
        // plays.
        if (melee_triggered) {
            // remove components to avoid further interactions
            reg.RemoveComponent<Component::HitBox>(reg.EntityFromIndex(eid));
            reg.RemoveComponent<Component::TimedEvents>(
                reg.EntityFromIndex(eid));
            reg.RemoveComponent<Component::FrameEvents>(
                reg.EntityFromIndex(eid));
            reg.RemoveComponent<Component::PatternMovement>(
                reg.EntityFromIndex(eid));
            if (reg.GetComponents<Component::PlayerTag>().has(eid))
                reg.RemoveComponent<Component::PlayerTag>(
                    reg.EntityFromIndex(eid));
            if (reg.GetComponents<Component::EnemyTag>().has(eid))
                reg.RemoveComponent<Component::EnemyTag>(
                    reg.EntityFromIndex(eid));
            // ensure AnimationDeath marker exists so it will be removed later
            reg.AddComponent<Component::AnimationDeath>(
                reg.EntityFromIndex(eid), Component::AnimationDeath{true});
        }
    }
}

}  // namespace server
