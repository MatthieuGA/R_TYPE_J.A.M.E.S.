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
    if (info.tag == "player") {
        CreatePlayerActor(entity, reg, info, is_local);
        return;
    }

    std::string tag_path = "";
    if (info.tag == "mermaid") {
        CreateMermaidActor(entity, reg, info);
        tag_path = "ennemies/4";
    }
    if (info.tag == "kamifish") {
        CreateKamiFishActor(entity, reg, info);
        tag_path = "ennemies/6";
    }
    CreateBasicEnnemy(tag_path, entity, reg, info);
}

void FactoryActors::CreateBasicActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::Transform>(
        entity, {0.f, 0.f, 0.0f, info.scale, Component::Transform::CENTER});
    reg.AddComponent<Component::Health>(
        entity, Component::Health(info.health));
    printf(
        "Creating actor '%s' with %d health\n", info.tag.c_str(), info.health);
    reg.AddComponent<Component::HealthBar>(entity,
        Component::HealthBar{
            sf::Vector2f(info.offset_healthbar.x, info.offset_healthbar.y)});
    reg.AddComponent<Component::HitBox>(
        entity, Component::HitBox{info.hitbox.x, info.hitbox.y});
    reg.AddComponent<Component::Velocity>(entity, Component::Velocity{});
    reg.AddComponent<Component::Drawable>(
        entity, Component::Drawable{info.spritePath, LAYER_ACTORS});
}

void FactoryActors::CreateBasicEnnemy(std::string const &tag_path,
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add basic enemy components
    reg.AddComponent<Component::EnemyTag>(
        entity, Component::EnemyTag{info.speed});

    // Add drawable and animated sprite components
    Component::AnimatedSprite animated_sprite(
        48, 48, 0.2f, true, sf::Vector2f(0.0f, 0.0f), 4);
    animated_sprite.AddAnimation(
        "Hit", tag_path + "/Hurt.png", 48, 48, 2, 0.1f, false);
    animated_sprite.AddAnimation(
        "Death", tag_path + "/Death.png", 48, 48, 6, 0.1f, false);
    animated_sprite.AddAnimation(
        "Attack", tag_path + "/Attack.png", 48, 48, 6, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));
}
}  // namespace Rtype::Client
