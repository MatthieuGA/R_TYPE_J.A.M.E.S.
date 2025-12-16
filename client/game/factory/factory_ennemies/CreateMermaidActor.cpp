#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateMermaidActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    sf::Vector2f(0.f, 50.f), sf::Vector2f(0.f, 1.f),
                    sf::Vector2f(0.f, 0.f), Rtype::Client::MERMAID_SPEED));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(
        Rtype::Client::MERMAID_PROJECTILE_SPEED,
        Rtype::Client::MERMAID_PROJECTILE_DAMAGE, sf::Vector2f(-3.0f, -15.0f));

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
