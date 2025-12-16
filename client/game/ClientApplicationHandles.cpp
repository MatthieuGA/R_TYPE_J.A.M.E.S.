#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "game/ClientApplication.hpp"
#include "game/InitRegistry.hpp"
#include "game/SnapshotTracker.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
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
    } else {
        printf("[Snapshot] Unknown projectile type 0x%02X for entity ID %u\n",
            entity_data.projectile_type, entity_data.entity_id);
    }
}

void ClientApplication::CreateNewEntity(GameWorld &game_world, int tick,
    const ClientApplication::ParsedEntity &entity_data,
    std::optional<size_t> &entity_index) {
    auto new_entity = game_world.registry_.SpawnEntity();
    if (entity_data.entity_type == 0x00) {
        // Player entity
        CreatePlayerEntity(game_world, new_entity, entity_data);
    } else if (entity_data.entity_type == 0x01) {
        // Enemy entity
        // For simplicity, create a basic enemy (e.g., "mermaid")
        // FactoryActors::GetInstance().CreateActor(
        //     new_entity, game_world.registry_, "mermaid", false);
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
}

void ClientApplication::UpdateExistingEntity(GameWorld &game_world,
    size_t entity_index, const ClientApplication::ParsedEntity &entity_data) {
    if (entity_data.entity_type == 0x00) {
        // Player entity
        UpdatePlayerEntity(game_world, entity_index, entity_data);
    } else if (entity_data.entity_type == 0x01) {
        // Enemy entity
        // Update logic can be added here if needed
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
