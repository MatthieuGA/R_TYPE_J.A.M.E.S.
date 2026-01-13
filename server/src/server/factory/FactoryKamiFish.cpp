#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

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

    reg.AddComponent<Component::ExplodeOnDeath>(
        entity, Component::ExplodeOnDeath{200.0f, 20});
}

}  // namespace server
