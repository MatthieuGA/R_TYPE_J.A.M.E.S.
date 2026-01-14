#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "game/SnapshotTracker.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

/**
 * @brief Handles the death of an entity by marking it for death animation
 * and removing relevant components.
 *
 * @param reg The registry.
 * @param game_world The game world (for audio access).
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param entity The entity to handle death for.
 * @param i Index of the entity in the registry.
 */
void DeathHandling(Engine::registry &reg, GameWorld &game_world,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::entity entity, std::size_t i) {
    // Check if this is a player and play death sound
    if (reg.GetComponents<Component::PlayerTag>().has(i)) {
        if (game_world.audio_manager_) {
            game_world.audio_manager_->PlaySound("player_death");
        }
    }

    // Check if this is a mermaid and play death sound
    auto &enemy_types = reg.GetComponents<Component::EnemyType>();
    if (enemy_types.has(i)) {
        auto &enemy_type = enemy_types[i];
        if (enemy_type.has_value() && enemy_type->type == "mermaid") {
            // Play mermaid death sound
            if (game_world.audio_manager_) {
                game_world.audio_manager_->PlaySound("mermaid_death");
            }
        }
        if (enemy_type.has_value() && enemy_type->type == "kamifish") {
            // Play kamifish death sound
            if (game_world.audio_manager_) {
                game_world.audio_manager_->PlaySound("kamifish_death");
            }
        }
    }

    // Mark entity with AnimationDeath component to trigger death anim
    reg.AddComponent<Component::AnimationDeath>(
        entity, Component::AnimationDeath{true});
    reg.RemoveComponent<Component::Health>(entity);
    reg.RemoveComponent<Component::HitBox>(entity);
    if (reg.GetComponents<Component::PlayerTag>().has(i))
        reg.RemoveComponent<Component::PlayerTag>(entity);
    if (reg.GetComponents<Component::EnemyTag>().has(i))
        reg.RemoveComponent<Component::EnemyTag>(entity);
    if (reg.GetComponents<Component::Projectile>().has(i))
        reg.RemoveComponent<Component::Projectile>(entity);
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
 * @brief System to remove entities that have not sent updates
 * for a certain number of ticks.
 *
 * This system checks the NetworkId component of each entity and
 * compares the last processed tick with the current tick from
 * SnapshotTracker. If the difference exceeds a defined threshold,
 * the entity is marked for removal by triggering its death handling.
 *
 * @param reg ECS registry used to access entities and components.
 * @param game_world The game world (for audio access).
 * @param network_ids Sparse array of NetworkId components.
 * @param animation_deaths Sparse array of AnimationDeath components.
 */
void KillEntitiesSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::NetworkId> &network_ids,
    Eng::sparse_array<Com::AnimationDeath> &animation_deaths) {
    const uint32_t kMaxTickDifference = 2;

    uint32_t current_tick =
        SnapshotTracker::GetInstance().GetLastProcessedTick();

    // Collect entities to kill first (avoid modifying registry while
    // iterating)
    std::vector<std::size_t> entities_to_kill;

    for (auto &&[i, net_id] : make_indexed_zipper(network_ids)) {
        if (animation_deaths.has(i)) {
            continue;  // Skip entities currently playing a death animation
        }
        // Safe subtraction check to avoid underflow issues
        if (current_tick > net_id.last_processed_tick &&
            (current_tick - net_id.last_processed_tick) > kMaxTickDifference) {
            entities_to_kill.push_back(i);
        }
    }

    // Now process the collected entities outside the iteration
    for (std::size_t i : entities_to_kill) {
        DeathHandling(reg, game_world,
            reg.GetComponents<Com::AnimatedSprite>(), reg.EntityFromIndex(i),
            i);
    }
}
}  // namespace Rtype::Client
