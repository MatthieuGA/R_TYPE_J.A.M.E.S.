#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateGolemActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        100, 100, 0.1f, true, sf::Vector2f(0.0f, 0.0f), 4);
    animated_sprite.AddAnimation("Idle", "ennemies/golem/golem_Boss.png", 100,
        100, 4, 0.1f, true, sf::Vector2f(0.0f, 0.0f));
    animated_sprite.AddAnimation("Death", "ennemies/golem/golem_Boss.png", 100,
        100, 14, 0.1f, false, sf::Vector2f(0.0f, 700.0f));
    animated_sprite.AddAnimation("Block", "ennemies/golem/golem_Boss.png", 100,
        100, 8, 0.1f, false, sf::Vector2f(0.0f, 300.0f));
    animated_sprite.AddAnimation("Attack", "ennemies/golem/golem_Boss.png",
        100, 100, 9, 0.1f, false, sf::Vector2f(0.0f, 200.0f));
    animated_sprite.AddAnimation("Smash", "ennemies/golem/golem_Boss.png", 100,
        100, 7, 0.1f, false, sf::Vector2f(0.0f, 400.0f));
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    sf::Vector2f(0.f, 50.f), sf::Vector2f(0.f, 1.f),
                    sf::Vector2f(0.f, 0.f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(info.speed,
        Rtype::Client::BASIC_PROJECTILE_DAMAGE, sf::Vector2f(-3.0f, -15.0f));
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}
}  // namespace Rtype::Client
