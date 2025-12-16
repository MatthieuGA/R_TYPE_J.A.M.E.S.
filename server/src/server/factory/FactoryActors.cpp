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

void FactoryActors::CreateMermaidProjectile(Engine::registry &reg,
    vector2f direction, Component::EnemyShootTag &enemy_shoot, int ownerId,
    Component::Transform const &transform) {
    auto projectile_entity = reg.SpawnEntity();
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{
            transform.x + (enemy_shoot.offset_shoot_position.x *
                              std::abs(transform.scale.x)),
            transform.y + (enemy_shoot.offset_shoot_position.y *
                              std::abs(transform.scale.y)),
            0.0f, 2.f, Component::Transform::CENTER});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{
            Component::Projectile::ProjectileType::Enemy_Mermaid,
            enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

void FactoryActors::CreatePlayerActor(Engine::entity &entity,
    Engine::registry &reg, EnnemyInfo info, bool is_local) {
    // Add player-specific components
    reg.AddComponent<Component::PlayerTag>(
        entity, Component::PlayerTag{400.0f});
    reg.AddComponent<Component::Controllable>(
        entity, Component::Controllable{});
}

void FactoryActors::CreateMermaidActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    vector2f(0.0f, 50.0f), vector2f(0.0f, 2.0f),
                    vector2f(-1.0f, 0.0f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-3.0f, -15.0f});

    // Add frame event for shooting
    reg.AddComponent<Component::FrameEvents>(
        entity, Component::FrameEvents("Attack", 5, [&reg](int entity_id) {
            try {
                auto &transform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id));
                auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id));

                vector2f shoot_direction = vector2f(-1.0f, 0.0f);
                FactoryActors::GetInstance().CreateMermaidProjectile(
                    reg, shoot_direction, enemy_shoot, entity_id, transform);
            } catch (const std::exception &e) {
                return;
            }
        }));

    // Add timed events for periodic attacks
    reg.AddComponent<Component::TimedEvents>(
        entity, Component::TimedEvents(
                    [&reg](int entity_id) {
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
                    2.0f));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
