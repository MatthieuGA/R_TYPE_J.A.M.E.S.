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

    // Add frame event with custom action
    reg.AddComponent<Component::FrameEvents>(entity,
        Component::FrameEvents("Attack", 5, [this, &reg](int entity_id) {
            // Custom action executed at frame 5 of Attack animation
            try {
                auto &transform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id));
                auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id));

                sf::Vector2f shoot_direction = sf::Vector2f(-1.0f, 0.0f);
                CreateEnemyProjectile(
                    reg, shoot_direction, enemy_shoot, entity_id, transform);
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
                    MERMAID_SHOOT_COOLDOWN));
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}
}  // namespace Rtype::Client
