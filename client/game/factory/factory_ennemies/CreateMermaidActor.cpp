#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateMermaidActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        48, 48, 0.2f, true, Engine::Graphics::Vector2f(0.0f, 0.0f), 4);
    animated_sprite.AddAnimation(
        "Hit", "ennemies/4/Hurt.png", 48, 48, 2, 0.1f, false);
    animated_sprite.AddAnimation(
        "Death", "ennemies/4/Death.png", 48, 48, 6, 0.1f, false);
    animated_sprite.AddAnimation(
        "Attack", "ennemies/4/Attack.png", 48, 48, 6, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    Engine::Graphics::Vector2f(0.f, 50.f),
                    Engine::Graphics::Vector2f(0.f, 1.f),
                    Engine::Graphics::Vector2f(0.f, 0.f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(info.speed,
        Rtype::Client::BASIC_PROJECTILE_DAMAGE,
        Engine::Graphics::Vector2f(-3.0f, -15.0f));

    // Add frame event with custom action
    reg.AddComponent<Component::FrameEvents>(entity,
        Component::FrameEvents("Attack", 5,
            [this, &reg](int entity_id) {
            // Custom action executed at frame 5 of Attack animation
            try {
                auto &transform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id));
                auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id));

                Engine::Graphics::Vector2f shoot_direction =
                    Engine::Graphics::Vector2f(-1.0f, 0.0f);
                // CreateMermaidProjectile(
                //     reg, shoot_direction, enemy_shoot, entity_id,
                //     transform);
            } catch (const std::exception &e) {
                return;
            }
        }));
    reg.AddComponent<Component::TimedEvents>(
        entity, Component::TimedEvents(
                    [this, &reg](int entity_id) {
                        // Default cooldown action: trigger Attack animation
                        try {
                            auto &animSprite =
                                reg.GetComponent<Component::AnimatedSprite>(
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
                    BASIC_SHOOT_COOLDOWN));
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}
}  // namespace Rtype::Client
