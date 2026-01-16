#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateInvActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        16, 16, 0.2f, true, Engine::Graphics::Vector2f(0.0f, 0.0f), 2);
    animated_sprite.AddAnimation(
        "Hit", "ennemies/powerups/PowerUp_Inv.png", 16, 16, 2, 0.1f, false);
    animated_sprite.AddAnimation("Death", "ennemies/powerups/Death_Inv_PU.png",
        16, 16, 4, 0.05f, false);
    animated_sprite.AddAnimation(
        "Attack", "ennemies/powerups/PowerUp_Inv.png", 16, 16, 2, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement());
    reg.AddComponent<Component::PowerUp>(entity, Component::PowerUp{});
}
}  // namespace Rtype::Client
