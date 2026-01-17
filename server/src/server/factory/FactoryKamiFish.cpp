#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

#include "include/indexed_zipper.tpp"

namespace fs = std::filesystem;

namespace server {

void FactoryActors::CreateKamiFishActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add pattern movement following the player
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-3.0f, -15.0f});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 4, 0.2f);
    animated_sprite.AddAnimation("Hit", 2, 0.1f, false);
    animated_sprite.AddAnimation("Death", 6, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 6, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Provide a custom explosion handler for KamiFish that applies AoE damage
    float radius = 200.0f;
    int damage = 20;
    Component::ExplodeOnDeath expl(radius, damage,
        [radius, damage](Engine::registry &reg, int exploding_entity_id,
            int /*collided_player_id*/) {
            auto &transforms = reg.GetComponents<Component::Transform>();
            auto &healths = reg.GetComponents<Component::Health>();

            if (!transforms.has(static_cast<std::size_t>(exploding_entity_id)))
                return;
            auto &t_self =
                transforms[static_cast<std::size_t>(exploding_entity_id)];
            float r2 = radius * radius;
            for (auto &&[tid, h] : make_indexed_zipper(healths)) {
                if (tid == static_cast<std::size_t>(exploding_entity_id))
                    continue;
                if (!transforms.has(tid))
                    continue;
                auto &t = transforms[tid];
                float dx = t_self->x - t->x;
                float dy = t_self->y - t->y;
                if (dx * dx + dy * dy <= r2) {
                    if (healths.has(tid)) {
                        auto &target_health = healths[tid];
                        if (target_health->invincibilityDuration > 0.0f)
                            continue;

                        target_health->currentHealth -= damage;
                        if (target_health->currentHealth < 0)
                            target_health->currentHealth = 0;
                    }
                }
            }
        });
    reg.AddComponent<Component::ExplodeOnDeath>(entity, std::move(expl));
}

}  // namespace server
