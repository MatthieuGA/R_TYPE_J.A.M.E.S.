#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateKamiFishActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        48, 48, 0.2f, true, sf::Vector2f(0.0f, 0.0f), 4);
    animated_sprite.AddAnimation(
        "Hit", "ennemies/6/Hurt.png", 48, 48, 2, 0.1f, false);
    animated_sprite.AddAnimation(
        "Death", "ennemies/6/Death.png", 48, 48, 6, 0.1f, false);
    animated_sprite.AddAnimation(
        "Attack", "ennemies/6/Attack.png", 48, 48, 6, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(info.speed));

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
