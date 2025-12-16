#include <iostream>
#include <utility>

#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

enum EntityType : uint8_t {
    PLAYER = 0,
    ENEMY = 1,
    PROJECTILE = 2
};

int GetEntityTypeFromRegistry(Engine::registry &registry_, size_t i) {
    if (registry_.GetComponents<Component::PlayerTag>().has(i)) {
        return EntityType::PLAYER;
    }
    if (registry_.GetComponents<Component::EnemyTag>().has(i)) {
        return EntityType::ENEMY;
    }
    if (registry_.GetComponents<Component::Projectile>().has(i)) {
        return EntityType::PROJECTILE;
    }
    // Add more component checks for other entity types as needed
    return -1;  // Unknown entity type
}

void SendServerSnapshotPlayer(network::EntityState &entity_state,
    Engine::registry &registry_, size_t i,
    Component::NetworkId &entity_network) {
    auto &transform = registry_.GetComponent<Component::Transform>(
        registry_.EntityFromIndex(i));
    entity_state.entity_id =
        network::EntityId{static_cast<uint32_t>(entity_network.id)};
    entity_state.entity_type = GetEntityTypeFromRegistry(registry_, i);
    entity_state.reserved = 0;

    // Normalize position to fit in 16 bits (0..65535 for x, 0..38864 for
    // y)
    entity_state.pos_x =
        static_cast<uint16_t>(std::clamp(transform.x, 0.0f, 65535.0f));
    entity_state.pos_y =
        static_cast<uint16_t>(std::clamp(transform.y, 0.0f, 38864.0f));
    try {
        auto &velocity = registry_.GetComponent<Component::Velocity>(
            registry_.EntityFromIndex(i));
        // Encode velocity with bias: [-32768, 32767] -> [0, 65535]
        int16_t vx_clamped =
            static_cast<int16_t>(std::clamp(velocity.vx, -32768.0f, 32767.0f));
        int16_t vy_clamped =
            static_cast<int16_t>(std::clamp(velocity.vy, -32768.0f, 32767.0f));
        entity_state.velocity_x =
            static_cast<uint16_t>(static_cast<int32_t>(vx_clamped) + 32768);
        entity_state.velocity_y =
            static_cast<uint16_t>(static_cast<int32_t>(vy_clamped) + 32768);
    } catch (const std::exception &e) {
        entity_state.velocity_x = 32768;  // 0 velocity encoded
        entity_state.velocity_y = 32768;
    }
    // Normalize angle to [0, 360) range
    float normalized_angle = transform.rotationDegrees;
    while (normalized_angle < 0.0f)
        normalized_angle += 360.0f;
    while (normalized_angle >= 360.0f)
        normalized_angle -= 360.0f;
    entity_state.angle = static_cast<uint16_t>(normalized_angle * 10.0f);
}

void SendServerSnapshotProjectile(network::EntityState &entity_state,
    Engine::registry &registry_, size_t i,
    Component::NetworkId &entity_network) {
    auto &transform = registry_.GetComponent<Component::Transform>(
        registry_.EntityFromIndex(i));
    entity_state.entity_id =
        network::EntityId{static_cast<uint32_t>(entity_network.id)};
    entity_state.entity_type = GetEntityTypeFromRegistry(registry_, i);
    entity_state.reserved = 0;
    // Normalize position to fit in 16 bits (0..65535 for x, 0..38864 for
    // y)
    entity_state.pos_x =
        static_cast<uint16_t>(std::clamp(transform.x, 0.0f, 65535.0f));
    entity_state.pos_y =
        static_cast<uint16_t>(std::clamp(transform.y, 0.0f, 38864.0f));
    // Normalize angle to [0, 360) range
    float normalized_angle = transform.rotationDegrees;
    while (normalized_angle < 0.0f)
        normalized_angle += 360.0f;
    while (normalized_angle >= 360.0f)
        normalized_angle -= 360.0f;
    entity_state.angle = static_cast<uint16_t>(normalized_angle * 10.0f);

    // Set projectile type if applicable
    try {
        auto &projectile = registry_.GetComponent<Component::Projectile>(
            registry_.EntityFromIndex(i));
        entity_state.projectile_type = static_cast<uint8_t>(projectile.type);
    } catch (const std::exception &e) {
        entity_state.projectile_type = 0;  // Default projectile type
    }
}

void Server::SendSnapshotsToAllClients() {
    uint32_t current_tick =
        tick_count_;  // Use same tick for all entities in this frame
    for (const auto &[i, entity_network] :
        make_indexed_zipper(registry_.GetComponents<Component::NetworkId>())) {
        network::EntityState entity_state;
        if (registry_.GetComponents<Component::PlayerTag>().has(i)) {
            SendServerSnapshotPlayer(
                entity_state, registry_, i, entity_network);
        } else if (registry_.GetComponents<Component::Projectile>().has(i)) {
            SendServerSnapshotProjectile(
                entity_state, registry_, i, entity_network);
        } else {
            // Skip entities that are neither players nor projectiles
            continue;
        }

        packet_sender_.SendSnapshot(entity_state, current_tick);
    }
    tick_count_++;  // Increment tick count once per frame, not per entity
}
}  // namespace server
