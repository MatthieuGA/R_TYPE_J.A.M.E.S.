#include "game/factory/factory_ennemies/FactoryActors.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace fs = std::filesystem;

namespace Rtype::Client {

void FactoryActors::CreateActor(Engine::entity &entity, Engine::registry &reg,
    std::string const &tag, bool is_local) {
    auto it = enemy_info_map_.find(tag);
    if (it == enemy_info_map_.end()) {
        std::cerr << "Enemy tag not found: " << tag << std::endl;
        return;
    }
    EnnemyInfo info = it->second;

    CreateBasicActor(entity, reg, info);
    if (tag == "powerup_invincibility") {
        CreateInvinsibilityActor(entity, reg);
        return;
    }

    if (info.tag == "player") {
        CreatePlayerActor(entity, reg, info, is_local);
        return;
    }

    CreateBasicEnnemy(entity, reg, info);
    if (info.tag == "mermaid")
        CreateMermaidActor(entity, reg, info);
    else if (info.tag == "kamifish")
        CreateKamiFishActor(entity, reg, info);
    else if (info.tag == "daemon")
        CreateDaemonActor(entity, reg, info);
}

void FactoryActors::CreateBasicActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::Transform>(
        entity, Component::Transform(0.f, 0.f, 0.0f,
                    Engine::Graphics::Vector2f(info.scale.x, info.scale.y),
                    Component::Transform::CENTER));
    reg.AddComponent<Component::Health>(
        entity, Component::Health(info.health));
    reg.AddComponent<Component::HealthBar>(entity,
        Component::HealthBar{
            sf::Vector2f(info.offset_healthbar.x, info.offset_healthbar.y)});
    reg.AddComponent<Component::HitBox>(
        entity, Component::HitBox{info.hitbox.x, info.hitbox.y});
    reg.AddComponent<Component::Velocity>(entity, Component::Velocity{});
    reg.AddComponent<Component::Drawable>(
        entity, Component::Drawable{info.spritePath, LAYER_ACTORS});
}

void FactoryActors::CreateBasicEnnemy(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::EnemyTag>(
        entity, Component::EnemyTag{info.speed});
}
}  // namespace Rtype::Client
