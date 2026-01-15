#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateDaemonActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        64, 64, 0.1f, true, Engine::Graphics::Vector2f(0.0f, 0.0f), 6);
    animated_sprite.AddAnimation("Hit", "ennemies/Daemon/Daemon_Sheet.png", 64,
        64, 4, 0.1f, false, Engine::Graphics::Vector2f(0.0f, 128.0f));
    animated_sprite.AddAnimation("Death", "ennemies/Daemon/Daemon_Sheet.png",
        64, 64, 8, 0.1f, false, Engine::Graphics::Vector2f(0.0f, 192.0f));
    animated_sprite.AddAnimation("Attack", "ennemies/Daemon/Daemon_Sheet.png",
        64, 64, 6, 0.1f, false, Engine::Graphics::Vector2f(0.0f, 64.0f));
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    Engine::Graphics::Vector2f(0.f, 50.f), Engine::Graphics::Vector2f(0.f, 1.f),
                    Engine::Graphics::Vector2f(0.f, 0.f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(info.speed,
        Rtype::Client::MERMAID_PROJECTILE_DAMAGE, sf::Vector2f(-7.0f, 8.0f));

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
                    MERMAID_SHOOT_COOLDOWN));
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}
}  // namespace Rtype::Client
