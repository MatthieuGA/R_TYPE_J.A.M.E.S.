#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateArchDemonActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        128, 128, 0.1f, true, Engine::Graphics::Vector2f(0.0f, 0.0f), 6);
    animated_sprite.AddAnimation("Idle",
        "ennemies/archdemon/ArchDemonIdle.png", 128, 128, 6, 0.1f, true,
        Engine::Graphics::Vector2f(0.0f, 0.0f));
    animated_sprite.AddAnimation("Death",
        "ennemies/archdemon/ArchDemonDeath.png", 128, 128, 8, 0.1f, false,
        Engine::Graphics::Vector2f(0.0f, 0.0f));
    animated_sprite.AddAnimation("Attack",
        "ennemies/archdemon/ArchDemonBasicAtk.png", 128, 128, 15, 0.1f, false,
        Engine::Graphics::Vector2f(0.0f, 0.0f));
    animated_sprite.AddAnimation("Attack2",
        "ennemies/archdemon/ArchDemonBasicAtk.png", 128, 128, 15, 0.1f, false,
        Engine::Graphics::Vector2f(0.0f, 0.0f));
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
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}
}  // namespace Rtype::Client
