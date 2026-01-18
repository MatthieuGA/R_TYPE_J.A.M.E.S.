#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

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
            enemy_shoot.speed_projectile * 2.f, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

void CreateGolemLaser(Engine::registry &reg, vector2f direction,
    Component::EnemyShootTag &enemy_shoot, int ownerId,
    Component::Transform const &transform) {
    auto projectile_entity = reg.SpawnEntity();
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{transform.x + (-30 * std::abs(transform.scale.x)),
            transform.y + (-15 * std::abs(transform.scale.y)), 0.0f, 2.f,
            Component::Transform::CENTER});
    Component::Projectile proj{
        Component::Projectile::ProjectileType::Enemy_Golem_Laser, 5, direction,
        enemy_shoot.speed_projectile, ownerId, true, 0.6f};
    proj.damage_mode = Component::Projectile::DamageMode::DamageOverTime;
    proj.tick_interval = 0.1f;  // damage every 0.1s
    proj.tick_timer = 0.0f;
    reg.AddComponent<Component::Projectile>(
        projectile_entity, std::move(proj));
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{1920.0f, 16.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{0, 0});
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
    // Add BossTag to identify this as a boss entity
    reg.AddComponent<Component::BossTag>(entity, Component::BossTag{"golem"});

    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    std::vector<vector2f>{{1700.f, 100.f}, {1700.f, 980.f}},
                    vector2f{0.f, 0.f}, info.speed, 0, true));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-3.0f, -15.0f},
        1000);  // score_value = 1000

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(4, 0.1f, true);
    animated_sprite.AddAnimation("Death", 14, 0.1f, false);
    animated_sprite.AddAnimation("Block", 8, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 9, 0.1f, false);
    animated_sprite.AddAnimation("Smash", 7, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add frame event for shooting
    Component::FrameEvents frame_events;

    frame_events.AddFrameEvent("Block", 6, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern with offset
            CircularShootPattern(reg, entity_id, 15.0f);
        } catch (const std::exception &e) {
            return;
        }
    });
    frame_events.AddFrameEvent("Block", 7, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern without offset
            CircularShootPattern(reg, entity_id, 0.0f);
        } catch (const std::exception &e) {
            return;
        }
    });
    frame_events.AddFrameEvent("Attack", 5, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern with larger offset
            CreateGolemLaser(reg, vector2f(0.0f, 0.0f),
                reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id)),
                entity_id,
                reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id)));
        } catch (const std::exception &e) {
            return;
        }
    });
    for (int j = 0; j < 2; j++) {
        int k = j * 3;
        frame_events.AddFrameEvent("Smash", 1 + k, [k, &reg](int entity_id) {
            try {
                for (int i = (k * 50); i < 1920; i += 400) {
                    auto &transform = reg.GetComponent<Component::Transform>(
                        reg.EntityFromIndex(entity_id));
                    auto &enemy_shoot =
                        reg.GetComponent<Component::EnemyShootTag>(
                            reg.EntityFromIndex(entity_id));

                    vector2f shoot_direction = vector2f(0.0f, 1.0f);
                    CreateGolemProjectile(reg, shoot_direction, enemy_shoot,
                        entity_id,
                        Component::Transform{static_cast<float>(i), 50.0f,
                            0.0f, 2.f, Component::Transform::CENTER});
                }
            } catch (const std::exception &e) {
                return;
            }
        });
    }
    reg.AddComponent<Component::FrameEvents>(entity, std::move(frame_events));

    // Add timed events for periodic attacks

    Component::TimedEvents timed_events;
    timed_events.AddCooldownAction(
        [&reg](int entity_id) {
            try {
                auto &animSprite = reg.GetComponent<Component::AnimatedSprite>(
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
        6.0f);
    timed_events.AddCooldownAction(
        [&reg](int entity_id) {
            try {
                auto &animSprite = reg.GetComponent<Component::AnimatedSprite>(
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
        6.0f, 2.f);
    timed_events.AddCooldownAction(
        [&reg](int entity_id) {
            try {
                auto &animSprite = reg.GetComponent<Component::AnimatedSprite>(
                    reg.EntityFromIndex(entity_id));
                auto &health = reg.GetComponent<Component::Health>(
                    reg.EntityFromIndex(entity_id));
                if (health.currentHealth <= 0)
                    return;
                animSprite.SetCurrentAnimation("Smash");
            } catch (const std::exception &e) {
                return;
            }
        },
        6.0f, 4.f);
    reg.AddComponent<Component::TimedEvents>(entity, std::move(timed_events));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
