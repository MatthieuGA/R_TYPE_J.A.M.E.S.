#include "server/factory/FactoryActors.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace fs = std::filesystem;

namespace server {

void FactoryActors::CreateActor(Engine::entity &entity, Engine::registry &reg,
    std::string const &tag, vector2f pos, bool is_local) {
    auto it = enemy_info_map_.find(tag);
    if (it == enemy_info_map_.end()) {
        std::cerr << "Enemy tag not found: " << tag << std::endl;
        return;
    }
    EnnemyInfo info = it->second;

    CreateBasicActor(pos, entity, reg, info);
    if (info.tag == "player") {
        CreatePlayerActor(entity, reg, info, is_local);
        return;
    }

    CreateBasicEnnemy(entity, reg, info);
    if (info.tag == "mermaid")
        CreateMermaidActor(entity, reg, info);
    if (info.tag == "kamifish")
        CreateKamiFishActor(entity, reg, info);
}

void FactoryActors::CreateBasicActor(vector2f pos, Engine::entity &entity,
    Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::Transform>(entity,
        {pos.x, pos.y, 0.0f, info.scale, Component::Transform::CENTER});
    reg.AddComponent<Component::Health>(
        entity, Component::Health(info.health));
    reg.AddComponent<Component::HitBox>(
        entity, Component::HitBox{info.hitbox.x, info.hitbox.y});
    reg.AddComponent<Component::Velocity>(entity, Component::Velocity{});
    reg.AddComponent<Component::NetworkId>(
        entity, Component::NetworkId{Server::GetNextNetworkId()});
}

void FactoryActors::CreateBasicEnnemy(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::EnemyTag>(
        entity, Component::EnemyTag{info.speed});

    // Set enemy subtype for network serialization
    uint8_t enemy_type_val = 0;
    if (info.tag == "kamifish")
        enemy_type_val = 1;
    else if (info.tag == "mermaid")
        enemy_type_val = 0;
    reg.AddComponent<Component::EnemyType>(
        entity, Component::EnemyType{enemy_type_val});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 4, 0.2f);
    animated_sprite.AddAnimation("Hit", 2, 0.1f, false);
    animated_sprite.AddAnimation("Death", 6, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 6, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));
}

void FactoryActors::CreatePlayerActor(Engine::entity &entity,
    Engine::registry &reg, EnnemyInfo info, bool is_local) {
    // Add player-specific components
    reg.AddComponent<Component::PlayerTag>(
        entity, Component::PlayerTag{400.0f});
    reg.AddComponent<Component::Controllable>(
        entity, Component::Controllable{});
}

}  // namespace server
