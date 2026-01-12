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

void CreateGolemProjectile(Engine::registry &reg, vector2f direction,
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
            Component::Projectile::ProjectileType::Enemy_Golem,
            enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

void CircularShootPattern(
    Engine::registry &reg, int entity_id, float angle_offset_deg = 0.0f) {
    auto &transform =
        reg.GetComponent<Component::Transform>(reg.EntityFromIndex(entity_id));
    auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
        reg.EntityFromIndex(entity_id));

    // Shoot in circular pattern
    for (float angle_deg = 0.0f; angle_deg < 360.0f; angle_deg += 30.0f) {
        float total_angle_deg = angle_deg + angle_offset_deg;
        float angle_rad = total_angle_deg * (3.14159265f / 180.0f);
        vector2f shoot_direction =
            vector2f(std::cos(angle_rad), std::sin(angle_rad));
        CreateGolemProjectile(
            reg, shoot_direction, enemy_shoot, entity_id, transform);
    }
}

void FactoryActors::CreateGolemActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    vector2f(0.0f, 50.0f), vector2f(0.0f, 2.0f),
                    vector2f(-1.0f, 0.0f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-3.0f, -15.0f});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 4, 0.2f);
    animated_sprite.AddAnimation("Death", 14, 0.1f, false);
    animated_sprite.AddAnimation("Block", 8, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add frame event for shooting
    Component::FrameEvents frame_events;

    frame_events.AddFrameEvent("Block", 7, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern with offset
            CircularShootPattern(reg, entity_id, 15.0f);
        } catch (const std::exception &e) {
            return;
        }
    });
    frame_events.AddFrameEvent("Block", 5, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern without offset
            CircularShootPattern(reg, entity_id, 0.0f);
        } catch (const std::exception &e) {
            return;
        }
    });
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
                            animSprite.SetCurrentAnimation("Block");
                        } catch (const std::exception &e) {
                            return;
                        }
                    },
                    5.0f));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
