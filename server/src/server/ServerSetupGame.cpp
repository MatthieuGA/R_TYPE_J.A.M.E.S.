#include <iostream>
#include <utility>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

constexpr const float MERMAID_SPEED = 50.0f;
constexpr const float MERMAID_SHOOT_COOLDOWN = 2.0f;
constexpr const int MERMAID_HEALTH = 30;
constexpr const float MERMAID_PROJECTILE_SPEED = 200.0f;
constexpr const float MERMAID_PROJECTILE_DAMAGE = 10.0f;

void CreateEnemyProjectile(Engine::registry &reg, vector2f direction,
    Component::EnemyShootTag &enemy_shoot, int ownerId,
    Component::Transform const &transform) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{
            transform.x + (enemy_shoot.offset_shoot_position.x *
                              std::abs(transform.scale.x)),
            transform.y + (enemy_shoot.offset_shoot_position.y *
                              std::abs(transform.scale.y)),
            0.0f, 2.f, Component::Transform::CENTER});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{Component::Projectile::ProjectileType::Enemy,
            enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
    // Assign a NetworkId so snapshots include this projectile
    reg.AddComponent<Component::NetworkId>(
        projectile_entity, Component::NetworkId{Server::GetNextNetworkId()});
}

void CreateMermaidActor(Engine::entity &entity, Engine::registry &reg) {
    // Add basic enemy components
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    {0.f, 50.f}, {0.f, 1.f}, {0.f, 0.f}, MERMAID_SPEED));
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    MERMAID_SPEED, 100.f, vector2f{600.f, 300.f}));
    reg.AddComponent<Component::Health>(
        entity, Component::Health(MERMAID_HEALTH));

    // loop, totalFrames, frameDuration
    Component::AnimatedSprite animated_sprite(true, 4, 0.2f);
    animated_sprite.AddAnimation("Hit", 2, 0.1f, false);
    animated_sprite.AddAnimation("Death", 6, 0.1f, false);
    animated_sprite.AddAnimation("Attack", 6, 0.15f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(
        MERMAID_PROJECTILE_SPEED, MERMAID_PROJECTILE_DAMAGE, {-3.0f, -15.0f});

    // Add frame event with custom action
    reg.AddComponent<Component::FrameEvents>(
        entity, Component::FrameEvents("Attack", 5, [&reg](int entity_id) {
            // Custom action executed at frame 5 of Attack animation
            try {
                auto &transform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id));
                auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id));

                vector2f shoot_direction = vector2f(-1.0f, 0.0f);
                CreateEnemyProjectile(
                    reg, shoot_direction, enemy_shoot, entity_id, transform);
            } catch (const std::exception &e) {
                return;
            }
        }));
    reg.AddComponent<Component::TimedEvents>(
        entity, Component::TimedEvents(
                    [&reg](int entity_id) {
                        // Default cooldown action: trigger Attack animation
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
                    MERMAID_SHOOT_COOLDOWN));
    reg.AddComponent<Component::EnemyShootTag>(
        entity, std::move(enemy_shoot_tag));
    reg.AddComponent<Component::HitBox>(
        entity, Component::HitBox{8.0f, 32.0f});
}

void Server::SetupEntitiesGame() {
    // Create one player entity per authenticated client, using the
    // client's assigned player_id as the NetworkId so clients can map
    // controlled entity easily.
    for (const auto &pair : connection_manager_.GetClients()) {
        const auto &client = pair.second;
        if (client.player_id_ == 0)
            continue;  // skip unauthenticated

        auto entity = registry_.spawn_entity();
        // Place players at distinct Y positions based on player_id
        float y = 200.0f * (static_cast<int>(client.player_id_) + 1);
        registry_.add_component(
            entity, Component::Transform{50.0f, y, 0.0f, {1.0f, 1.0f}});
        registry_.add_component(entity, Component::Velocity{1.0f, 0.0f});
        registry_.add_component(entity, Component::PlayerTag{400.0f});
        registry_.add_component(entity, Component::Controllable{});
        registry_.add_component(entity, Component::HitBox{30.0f, 10.0f});
        registry_.add_component(entity, Component::Health{100});
        registry_.add_component(
            entity, Component::NetworkId{Server::GetNextNetworkId()});
        std::cout << "Created entity for Player "
                  << static_cast<int>(client.player_id_) << std::endl;
    }

    // Spawns of enemies/projectiles
    auto enemy = registry_.spawn_entity();
    registry_.add_component(
        enemy, Component::Transform{600.0f, 300.0f, 0.0f, {1.0f, 1.0f}});
    registry_.add_component(enemy, Component::EnemyTag{});
    registry_.add_component(enemy, Component::Velocity{});
    registry_.add_component(
        enemy, Component::NetworkId{Server::GetNextNetworkId()});
    CreateMermaidActor(enemy, registry_);
    std::cout << "Created enemy entity" << std::endl;
}
}  // namespace server
