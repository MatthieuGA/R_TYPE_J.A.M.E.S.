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

void CreateArchdemonProjectile(Engine::registry &reg, vector2f direction,
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
            Component::Projectile::ProjectileType::Enemy_ArchDemon,
            enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{10.0f, 10.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
}

void FactoryActors::CreateArchDemonActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add BossTag to identify this as a boss entity
    reg.AddComponent<Component::BossTag>(
        entity, Component::BossTag{"archdemon"});

    // Add pattern movement
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    std::vector<vector2f>{{1700.f, 100.f}, {1700.f, 980.f}},
                    vector2f{0.f, 0.f}, info.speed, 0, true));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(100.0f, 10.0f, {-3.0f, -15.0f});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 6, 0.1f);
    animated_sprite.AddAnimation("Death", 8, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 15, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add frame event for shooting
    Component::FrameEvents frame_events;

    frame_events.AddFrameEvent("Attack", 9, [&reg](int entity_id) {
        try {
            // Shoot in circular pattern with larger offset
            CreateArchdemonProjectile(reg, vector2f(0.0f, 0.0f),
                reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id)),
                entity_id,
                reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id)));
        } catch (const std::exception &e) {
            return;
        }
    });
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
                animSprite.SetCurrentAnimation("Attack");
            } catch (const std::exception &e) {
                return;
            }
        },
        6.0f, 2.f);
    reg.AddComponent<Component::TimedEvents>(entity, std::move(timed_events));

    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
}

}  // namespace server
