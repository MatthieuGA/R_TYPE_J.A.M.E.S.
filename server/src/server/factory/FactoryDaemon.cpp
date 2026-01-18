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

void CreateDaemonProjectile(Engine::registry &reg, vector2f direction,
    Component::EnemyShootTag &enemy_shoot, int ownerId,
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
            Component::Projectile::ProjectileType::Enemy_Daemon,
            enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

void Attack(Engine::registry &reg, int entity_id) {
    try {
        auto &transform = reg.GetComponent<Component::Transform>(
            reg.EntityFromIndex(entity_id));
        auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
            reg.EntityFromIndex(entity_id));

        vector2f shoot_direction = vector2f(-1.0f, 0.0f);
        CreateDaemonProjectile(
            reg, shoot_direction, enemy_shoot, entity_id, transform);
        shoot_direction = vector2f(-0.866f, 0.5f);
        CreateDaemonProjectile(
            reg, shoot_direction, enemy_shoot, entity_id, transform);
        shoot_direction = vector2f(-0.866f, -0.5f);
        CreateDaemonProjectile(
            reg, shoot_direction, enemy_shoot, entity_id, transform);
    } catch (const std::exception &e) {
        return;
    }
}

void FactoryActors::CreateDaemonActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    vector2f(0.0f, 50.0f), vector2f(0.0f, 2.0f),
                    vector2f(-1.0f, 0.0f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-8.0f, 8.0f},
        166);  // score_value = 166

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 6, 0.1f);
    animated_sprite.AddAnimation("Hit", 4, 0.1f, false);
    animated_sprite.AddAnimation("Death", 8, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 6, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add frame event for shooting

    Component::FrameEvents frame_events;
    frame_events.AddFrameEvent(
        "Attack", 5, [&reg](int entity_id) { Attack(reg, entity_id); });
    frame_events.AddFrameEvent(
        "Attack", 1, [&reg](int entity_id) { Attack(reg, entity_id); });
    reg.AddComponent<Component::FrameEvents>(entity, std::move(frame_events));

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
                    4.0f));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
