#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "game/SnapshotTracker.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void DeathHandling(Engine::registry &reg,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
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

void KillEntitiesSystem(Eng::registry &reg,
    Eng::sparse_array<Com::NetworkId> &network_ids,
    Eng::sparse_array<Com::AnimationDeath> &animation_deaths) {
    const int kMaxTickDifference = 2;

    for (auto &&[i, net_id] : make_indexed_zipper(network_ids)) {
        if (animation_deaths.has(i)) {
            continue;  // Skip entities currently playing a death animation
        }
        if (SnapshotTracker::GetInstance().GetLastProcessedTick() -
                static_cast<uint32_t>(net_id.last_processed_tick) >
            kMaxTickDifference) {
            DeathHandling(reg, reg.GetComponents<Com::AnimatedSprite>(),
                reg.EntityFromIndex(i), i);
        }
    }
}
}  // namespace Rtype::Client
