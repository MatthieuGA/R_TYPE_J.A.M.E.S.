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
    try {
        auto &health = game_world.registry_
                           .GetComponents<Component::Health>()[entity_index];
        if (health.has_value()) {
            health->currentHealth = static_cast<int>(entity_data.health);
        }
    } catch (const std::exception &e) {
        // Health component might not exist; ignore if so
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
        auto &health = game_world.registry_
                           .GetComponents<Component::Health>()[entity_index];
        if (health.has_value()) {
            health->currentHealth = static_cast<int>(entity_data.health);
        }
    } catch (const std::exception &e) {
        // Health component might not exist; ignore if so
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
    } else if (entity_data.entity_type == 0x03) {
        // Obstacle entity - update like enemies (position, velocity)
        UpdateEnemyEntity(game_world, entity_index, entity_data);
    } else {
        printf("[Snapshot] Unknown entity type 0x%02X for entity ID %u\n",
            entity_data.entity_type, entity_data.entity_id);
    }
}

}  // namespace Rtype::Client
