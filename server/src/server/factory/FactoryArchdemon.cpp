#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

namespace fs = std::filesystem;

namespace server {

void FactoryActors::CreateArchDemonActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add BossTag to identify this as a boss entity
    reg.AddComponent<Component::BossTag>(
        entity, Component::BossTag{"archdemon"});

    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    std::vector<vector2f>{{1700.f, 100.f}, {1700.f, 980.f}},
                    vector2f{0.f, 0.f}, info.speed, 0, true));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(100.0f, 10.0f, {-3.0f, -15.0f});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(6, 0.1f, true);
    animated_sprite.AddAnimation("Idle", 6, 0.1f, false);
    animated_sprite.AddAnimation("Death", 8, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 15, 0.1f, false);
    animated_sprite.AddAnimation("Attack2", 15, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add frame event for shooting
    Component::FrameEvents frame_events;
    reg.AddComponent<Component::FrameEvents>(entity, std::move(frame_events));

    // Add timed events for periodic attacks

    Component::TimedEvents timed_events;
    timed_events.AddCooldownAction(
        [&reg](int entity_id) {
            try {
                auto &animSprite = reg.GetComponent<Component::AnimatedSprite>(
                    reg.EntityFromIndex(entity_id));
                auto &health = reg.GetComponent<Component::Health>(
                    reg.EntityFromIndex(entity_id));
                if (health.currentHealth <= 0)
                    return;
                animSprite.SetCurrentAnimation("Attack");
            } catch (const std::exception &e) {
                return;
            }
        },
        6.0f, 2.f);
    timed_events.AddCooldownAction(
        [&reg](int entity_id) {
            try {
                auto &animSprite = reg.GetComponent<Component::AnimatedSprite>(
                    reg.EntityFromIndex(entity_id));
                auto &health = reg.GetComponent<Component::Health>(
                    reg.EntityFromIndex(entity_id));
                if (health.currentHealth <= 0)
                    return;
                animSprite.SetCurrentAnimation("Attack");
            } catch (const std::exception &e) {
                return;
            }
        },
        6.0f, 4.f);
    reg.AddComponent<Component::TimedEvents>(entity, std::move(timed_events));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
