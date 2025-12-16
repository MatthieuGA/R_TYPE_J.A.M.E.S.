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

    printf("[Snapshot] Creating enemy entity ID %u at position (%u, %u)\n",
        entity_data.entity_id, entity_data.pos_x, entity_data.pos_y);
    FactoryActors::GetInstance().CreateActor(
        new_entity, game_world.registry_, "mermaid", false);
    try {
        printf(
            "[Snapshot] Setting enemy animation %u frame %u for entity ID "
            "%u\n",
            entity_data.current_animation, entity_data.current_frame,
            entity_data.entity_id);
        auto &animated_sprite = game_world.registry_.GetComponents<
            Component::AnimatedSprite>()[new_entity.GetId()];
        if (animated_sprite.has_value()) {
            printf(
                "[Snapshot] Found AnimatedSprite component for entity ID %u\n",
                entity_data.entity_id);
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
            printf(
                "[Snapshot] Set animation '%s' and frame for entity ID %u\n",
                chosen.c_str(), entity_data.entity_id);
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

static void CreateProjectileEntity(GameWorld &game_world,
    Engine::registry::entity_t new_entity,
    const ClientApplication::ParsedEntity &entity_data) {
    // Projectile entity
    if (entity_data.projectile_type == 0x00) {
        // Basic projectile
        createProjectile(game_world.registry_,
            static_cast<float>(entity_data.pos_x),
            static_cast<float>(entity_data.pos_y), /*ownerId=*/-1, new_entity);
    } else if (entity_data.projectile_type == 0x01) {
        // Charged projectile
        createChargedProjectile(game_world.registry_,
            static_cast<float>(entity_data.pos_x),
            static_cast<float>(entity_data.pos_y), /*ownerId=*/-1, new_entity);
    } else if (entity_data.projectile_type == 0x02) {
        // Charged projectile DEEBBBBBUUUGGGG
        CreateMermaidProjectile(game_world.registry_, entity_data, new_entity);
    } else {
        printf("[Snapshot] Unknown projectile type 0x%02X for entity ID %u\n",
            entity_data.projectile_type, entity_data.entity_id);
    }
}

void ClientApplication::CreateNewEntity(GameWorld &game_world, int tick,
    const ClientApplication::ParsedEntity &entity_data,
    std::optional<size_t> &entity_index) {
    auto new_entity = game_world.registry_.SpawnEntity();
    printf("[Snapshot] Creating new entity of type 0x%02X ...\n",
        entity_data.entity_type);
    if (entity_data.entity_type == 0x00) {
        // Player entity
        CreatePlayerEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type == 0x01) {
        // Enemy entity
        // For simplicity, create a basic enemy (e.g., "mermaid")
        CreateEnemyEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type == 0x02) {
        // Projectile entity
        CreateProjectileEntity(game_world, new_entity, entity_data);
    } else {
        printf("[Snapshot] Unknown entity type 0x%02X for entity ID %u\n",
            entity_data.entity_type, entity_data.entity_id);
        return;
    }

    game_world.registry_.AddComponent<Component::NetworkId>(new_entity,
        Component::NetworkId{static_cast<int>(entity_data.entity_id), tick});
    size_t entity_id = new_entity.GetId();
    entity_index = entity_id;

    std::cout << "[Snapshot] Created entity #" << entity_id
              << " for NetworkId=" << entity_data.entity_id << "at tick "
              << tick << std::endl;
}

static void UpdatePlayerEntity(GameWorld &game_world, size_t entity_index,
    const ClientApplication::ParsedEntity &entity_data) {
    // Update existing entity's transform
    if (!game_world.registry_.GetComponents<Component::Transform>().has(
            entity_index))
        return;
    auto &transform = game_world.registry_
                          .GetComponents<Component::Transform>()[entity_index];
    if (transform.has_value()) {
        transform->x = static_cast<float>(entity_data.pos_x);
        transform->y = static_cast<float>(entity_data.pos_y);
        transform->rotationDegrees =
            static_cast<float>(entity_data.angle / 10.0f);
    }
    try {
        auto &velocity =
            game_world.registry_
                .GetComponents<Component::Velocity>()[entity_index];
        if (velocity.has_value()) {
            // Decode velocity from bias encoding: [0, 65535] -> [-32768,
            // 32767]
            velocity->vx = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_x) - 32768);
            velocity->vy = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_y) - 32768);
        }
    } catch (const std::exception &e) {
        // Velocity component might not exist; ignore if so
    }
}

static void UpdateEnemyEntity(GameWorld &game_world, size_t entity_index,
    const ClientApplication::ParsedEntity &entity_data) {
    // Update existing entity's transform
    if (!game_world.registry_.GetComponents<Component::Transform>().has(
            entity_index))
        return;
    auto &transform = game_world.registry_
                          .GetComponents<Component::Transform>()[entity_index];
    if (transform.has_value()) {
        transform->x = static_cast<float>(entity_data.pos_x);
        transform->y = static_cast<float>(entity_data.pos_y);
        transform->rotationDegrees =
            static_cast<float>(entity_data.angle / 10.0f);
    }
    try {
        auto &velocity =
            game_world.registry_
                .GetComponents<Component::Velocity>()[entity_index];
        if (velocity.has_value()) {
            // Decode velocity from bias encoding: [0, 65535] -> [-32768,
            // 32767]
            velocity->vx = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_x) - 32768);
            velocity->vy = static_cast<float>(
                static_cast<int32_t>(entity_data.velocity_y) - 32768);
        }
    } catch (const std::exception &e) {
        // Velocity component might not exist; ignore if so
    }

    try {
        auto &animated_sprite =
            game_world.registry_
                .GetComponents<Component::AnimatedSprite>()[entity_index];
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
        // AnimatedSprite component might not exist; ignore if so
    }
}

static void UpdateProjectileEntity(GameWorld &game_world, size_t entity_index,
    const ClientApplication::ParsedEntity &entity_data) {
    // Update existing entity's transform
    if (!game_world.registry_.GetComponents<Component::Transform>().has(
            entity_index)) {
        return;
    }
    auto &transform = game_world.registry_
                          .GetComponents<Component::Transform>()[entity_index];
    if (transform.has_value()) {
        transform->x = static_cast<float>(entity_data.pos_x);
        transform->y = static_cast<float>(entity_data.pos_y);
        transform->rotationDegrees =
            static_cast<float>(entity_data.angle / 10.0f);
    }
    // Decode and update velocity
    try {
        int16_t decoded_vx =
            static_cast<int16_t>(entity_data.velocity_x) - 32768;
        int16_t decoded_vy =
            static_cast<int16_t>(entity_data.velocity_y) - 32768;
        auto &velocity =
            game_world.registry_
                .GetComponents<Component::Velocity>()[entity_index];
        if (velocity.has_value()) {
            velocity->vx = static_cast<float>(decoded_vx);
            velocity->vy = static_cast<float>(decoded_vy);
        }
    } catch (const std::exception &e) {
        // Velocity component might not exist; ignore if so
    }
}

void ClientApplication::UpdateExistingEntity(GameWorld &game_world,
    size_t entity_index, const ClientApplication::ParsedEntity &entity_data) {
    if (entity_data.entity_type == 0x00) {
        // Player entity
        UpdatePlayerEntity(game_world, entity_index, entity_data);
    } else if (entity_data.entity_type == 0x01) {
        // Enemy entity
        // Update logic can be added here if needed
        UpdateEnemyEntity(game_world, entity_index, entity_data);
    } else if (entity_data.entity_type == 0x02) {
        // Projectile entity
        // Projectiles are usually short-lived; may not need updates
        UpdateProjectileEntity(game_world, entity_index, entity_data);
    } else {
        printf("[Snapshot] Unknown entity type 0x%02X for entity ID %u\n",
            entity_data.entity_type, entity_data.entity_id);
    }
}

}  // namespace Rtype::Client
