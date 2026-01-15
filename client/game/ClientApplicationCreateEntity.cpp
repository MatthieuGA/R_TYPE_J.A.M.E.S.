#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "game/ClientApplication.hpp"
#include "game/InitRegistry.hpp"
#include "game/SnapshotTracker.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "network/Network.hpp"

namespace Rtype::Client {
// Parse Player

static void CreatePlayerEntity(GameWorld &game_world,
    Engine::registry::entity_t new_entity,
    const ClientApplication::ParsedEntity &entity_data) {
    // Player entity
    bool is_local = false;
    if (game_world.server_connection_ &&
        game_world.server_connection_->is_connected()) {
        uint32_t controlled =
            game_world.server_connection_->controlled_entity_id();
        if (controlled != 0 && controlled == entity_data.entity_id)
            is_local = true;
    }

    FactoryActors::GetInstance().CreateActor(
        new_entity, game_world.registry_, "player", is_local);
}

static void CreateEnemyEntity(GameWorld &game_world,
    Engine::registry::entity_t new_entity,
    const ClientApplication::ParsedEntity &entity_data) {
    // Player entity

    std::string enemy_type_str = "unknown";
    if (entity_data.enemy_type ==
        ClientApplication::ParsedEntity::kKamiFishEnemy) {
        enemy_type_str = "kamifish";
    } else if (entity_data.enemy_type ==
               ClientApplication::ParsedEntity::kMermaidEnemy) {
        enemy_type_str = "mermaid";
    } else if (entity_data.enemy_type ==
               ClientApplication::ParsedEntity::kDaemonEnemy) {
        enemy_type_str = "daemon";
    }
    FactoryActors::GetInstance().CreateActor(
        new_entity, game_world.registry_, enemy_type_str, false);

    // Add EnemyType component to identify enemy type later
    game_world.registry_.AddComponent<Component::EnemyType>(
        new_entity, Component::EnemyType(enemy_type_str));

    try {
        auto &animated_sprite = game_world.registry_.GetComponents<
            Component::AnimatedSprite>()[new_entity.GetId()];
        if (animated_sprite.has_value()) {
            const auto names = animated_sprite->GetAnimationNames();
            std::string chosen = "Default";
            if (entity_data.current_animation < names.size())
                chosen = names[entity_data.current_animation];
            animated_sprite->SetCurrentAnimation(chosen);
            auto it = animated_sprite->animations.find(chosen);
            if (it != animated_sprite->animations.end()) {
                int total = std::max(0, it->second.totalFrames);
                int frame = static_cast<int>(entity_data.current_frame);
                if (total > 0) {
                    if (frame < 0)
                        frame = 0;
                    if (frame >= total)
                        frame = total - 1;
                } else {
                    frame = 0;
                }
                it->second.current_frame = frame;
            }
        }
    } catch (const std::exception &e) {
        printf("[Snapshot] ERROR setting animation for entity ID %u: %s\n",
            entity_data.entity_id, e.what());
    }
}

void CreateMermaidProjectile(Engine::registry &reg,
    const ClientApplication::ParsedEntity &entity_data,
    Engine::registry::entity_t new_entity) {
    // Decode velocity from encoded format (encoded = actual + 32768)
    int16_t decoded_vx = static_cast<int16_t>(entity_data.velocity_x) - 32768;
    int16_t decoded_vy = static_cast<int16_t>(entity_data.velocity_y) - 32768;

    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(
        new_entity, Component::Transform{static_cast<float>(entity_data.pos_x),
                        static_cast<float>(entity_data.pos_y), 0.0f, 2.f,
                        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(new_entity,
        Component::Drawable("ennemies/4/Projectile.png", LAYER_PROJECTILE));
    reg.AddComponent<Component::Projectile>(new_entity,
        Component::Projectile{static_cast<int>(MERMAID_PROJECTILE_DAMAGE),
            sf::Vector2f(decoded_vx, decoded_vy), MERMAID_PROJECTILE_SPEED, -1,
            true});
    reg.AddComponent<Component::HitBox>(
        new_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        new_entity, Component::Velocity{static_cast<float>(decoded_vx),
                        static_cast<float>(decoded_vy)});
    reg.AddComponent<Component::ParticleEmitter>(new_entity,
        Component::ParticleEmitter(50, 50, RED_HIT, RED_HIT,
            sf::Vector2f(0.f, 0.f), true, 0.3f, 4.f, sf::Vector2f(-1.f, 0.f),
            45.f, 0, 8, 3.0f, 2.0f, -1.0f, LAYER_PARTICLE));
}

void CreateDaemonProjectile(Engine::registry &reg,
    const ClientApplication::ParsedEntity &entity_data,
    Engine::registry::entity_t new_entity) {
    // Decode velocity from encoded format (encoded = actual + 32768)
    int16_t decoded_vx = static_cast<int16_t>(entity_data.velocity_x) - 32768;
    int16_t decoded_vy = static_cast<int16_t>(entity_data.velocity_y) - 32768;

    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(
        new_entity, Component::Transform{static_cast<float>(entity_data.pos_x),
                        static_cast<float>(entity_data.pos_y), 0.0f, 2.f,
                        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        new_entity, Component::Drawable(
                        "ennemies/Daemon/Projectile.png", LAYER_PROJECTILE));
    reg.AddComponent<Component::Projectile>(new_entity,
        Component::Projectile{static_cast<int>(MERMAID_PROJECTILE_DAMAGE),
            sf::Vector2f(decoded_vx, decoded_vy), MERMAID_PROJECTILE_SPEED, -1,
            true});
    reg.AddComponent<Component::HitBox>(
        new_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        new_entity, Component::Velocity{static_cast<float>(decoded_vx),
                        static_cast<float>(decoded_vy)});
    reg.AddComponent<Component::ParticleEmitter>(new_entity,
        Component::ParticleEmitter(50, 50, ORANGE_HIT, ORANGE_HIT,
            sf::Vector2f(0.f, 0.f), true, 0.3f, 4.f, sf::Vector2f(-1.f, 0.f),
            45.f, 0, 8, 3.0f, 2.0f, -1.0f, LAYER_PARTICLE));
}

static void CreateProjectileEntity(GameWorld &game_world,
    Engine::registry::entity_t new_entity,
    const ClientApplication::ParsedEntity &entity_data) {
    // Projectile entity
    if (entity_data.projectile_type ==
        ClientApplication::ParsedEntity::kPlayerProjectile) {
        // Basic projectile
        if (game_world.audio_manager_) {
            game_world.audio_manager_->PlaySound("player_shot");
        }
        CreateProjectile(game_world.registry_,
            static_cast<float>(entity_data.pos_x),
            static_cast<float>(entity_data.pos_y), /*ownerId=*/-1, new_entity);
    } else if (entity_data.projectile_type ==
               ClientApplication::ParsedEntity::kPlayerChargedProjectile) {
        // Charged projectile
        if (game_world.audio_manager_) {
            game_world.audio_manager_->PlaySound("charged_shot");
        }
        createChargedProjectile(game_world.registry_,
            static_cast<float>(entity_data.pos_x),
            static_cast<float>(entity_data.pos_y), /*ownerId=*/-1, new_entity);
    } else if (entity_data.projectile_type ==
               ClientApplication::ParsedEntity::kMermaidProjectile) {
        // Mermaid projectile
        if (game_world.audio_manager_) {
            game_world.audio_manager_->PlaySound("small_shot");
        }
        CreateMermaidProjectile(game_world.registry_, entity_data, new_entity);
    } else if (entity_data.projectile_type ==
               ClientApplication::ParsedEntity::kDaemonProjectile) {
        // Mermaid projectile
        CreateDaemonProjectile(game_world.registry_, entity_data, new_entity);
    } else {
        printf("[Snapshot] Unknown projectile type 0x%02X for entity ID %u\n",
            entity_data.projectile_type, entity_data.entity_id);
    }
}

/**
 * @brief Creates a visual obstacle entity on the client side.
 *
 * Obstacles are solid world objects that move with the world scroll.
 * They block player movement and can crush players against the screen edge.
 *
 * @param game_world The game world containing the registry
 * @param new_entity The newly spawned entity
 * @param entity_data Network data for the entity
 */
static void CreateObstacleEntity(GameWorld &game_world,
    Engine::registry::entity_t new_entity,
    const ClientApplication::ParsedEntity &entity_data) {
    // Decode velocity from encoded format (encoded = actual + 32768)
    int16_t decoded_vx = static_cast<int16_t>(entity_data.velocity_x) - 32768;
    int16_t decoded_vy = static_cast<int16_t>(entity_data.velocity_y) - 32768;

    // Obstacle size - must match server default (32x32)
    // Server's WorldGenConfigLoader defaults to 32x32 when not specified in WGF
    // This ensures collision detection works consistently
    constexpr float kObstacleWidth = 32.0f;
    constexpr float kObstacleHeight = 32.0f;

    // Visual offset to align the debug rectangle with the collision hitbox
    // The player sprite (34x18 @ 4x scale) has its visual center slightly
    // offset from its hitbox center (30x10 @ 4x scale), so we add a small
    // offset to the obstacle visual to match better
    constexpr float kVisualOffsetX = 0.0f;
    constexpr float kVisualOffsetY = 0.0f;

    // Add Transform component
    // Scale 1.0 - hitbox will define the collision size
    game_world.registry_.AddComponent<Component::Transform>(
        new_entity, Component::Transform{static_cast<float>(entity_data.pos_x),
                        static_cast<float>(entity_data.pos_y), 0.0f, 1.0f,
                        Component::Transform::CENTER});

    // Add RectangleDrawable - simple red box for obstacles
    // Render behind players (LAYER_ACTORS - 1)
    game_world.registry_.AddComponent<Component::RectangleDrawable>(new_entity,
        Component::RectangleDrawable{
            kObstacleWidth, kObstacleHeight,
            Engine::Graphics::Color(180, 40, 40, 255),  // Dark red fill
            Engine::Graphics::Color(255, 80, 80, 255),  // Lighter red outline
            2.0f,                                       // Outline thickness
            LAYER_ACTORS - 1,                           // z_index (behind players)
            kVisualOffsetX,                             // Visual X offset
            kVisualOffsetY                              // Visual Y offset
        });

    // Add Velocity for interpolation
    game_world.registry_.AddComponent<Component::Velocity>(
        new_entity, Component::Velocity{static_cast<float>(decoded_vx),
                        static_cast<float>(decoded_vy)});

    // Add HitBox matching the obstacle's visual size
    game_world.registry_.AddComponent<Component::HitBox>(
        new_entity, Component::HitBox{kObstacleWidth, kObstacleHeight});

    // Add Solid component - obstacles are solid and locked (cannot be pushed)
    game_world.registry_.AddComponent<Component::Solid>(new_entity,
        Component::Solid{true, true});  // isSolid=true, isLocked=true
}

void ClientApplication::CreateNewEntity(GameWorld &game_world, uint32_t tick,
    const ClientApplication::ParsedEntity &entity_data,
    std::optional<size_t> &entity_index) {
    auto new_entity = game_world.registry_.SpawnEntity();

    if (entity_data.entity_type ==
        ClientApplication::ParsedEntity::kPlayerEntity) {
        // Player entity
        CreatePlayerEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type ==
               ClientApplication::ParsedEntity::kEnemyEntity) {
        // Enemy entity
        CreateEnemyEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type ==
               ClientApplication::ParsedEntity::kProjectileEntity) {
        // Projectile entity
        CreateProjectileEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type ==
               ClientApplication::ParsedEntity::kObstacleEntity) {
        // Obstacle entity (asteroids, walls, etc.)
        CreateObstacleEntity(game_world, new_entity, entity_data);
    } else {
        printf("[Snapshot] Unknown entity type 0x%02X for entity ID %u\n",
            entity_data.entity_type, entity_data.entity_id);
        return;
    }

    game_world.registry_.AddComponent<Component::NetworkId>(new_entity,
        Component::NetworkId{static_cast<int>(entity_data.entity_id), tick});
    entity_index = new_entity.GetId();
}

}  // namespace Rtype::Client
