#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "include/registry.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"

// #include "server/Packets.hpp"  // TODO: Create Packets.hpp with packet type
// definitions

// Forward declarations for network types (temporary until Packets.hpp is
// created)
namespace network {
using Tick = uint32_t;
using EntityId = uint32_t;
using PlayerId = uint8_t;

struct SpawnEntityPacket {
    Tick tick;
    EntityId entity_id;
    uint8_t entity_type;
    float pos_x, pos_y;
    float vel_x, vel_y;
};

struct PlayerShootPacket {
    Tick tick;
    PlayerId player_id;
    float pos_x, pos_y;
    float angle;
};
}  // namespace network

namespace server::converters {

/**
 * @brief Convert ECS components to network packets
 *
 * These functions create packets from component data.
 * They define the mapping between game state (ECS) and network protocol.
 */

// ============================================================================
// COMPONENT → PACKET CONVERTERS
// ============================================================================

/**
 * @brief Create a SpawnEntity packet from ECS components
 */
inline network::SpawnEntityPacket ToSpawnEntityPacket(network::Tick tick,
    network::EntityId entity_id, uint8_t entity_type,
    const Component::Transform &pos, const Component::Velocity &vel) {
    network::SpawnEntityPacket packet;
    packet.tick = tick;
    packet.entity_id = entity_id;
    packet.entity_type = entity_type;
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    packet.vel_x = vel.vx;
    packet.vel_y = vel.vy;
    return packet;
}

/**
 * @brief Create a PlayerShoot packet from position component
 */
inline network::PlayerShootPacket ToPlayerShootPacket(network::Tick tick,
    network::PlayerId player_id, const Component::Transform &pos,
    float angle) {
    network::PlayerShootPacket packet;
    packet.tick = tick;
    packet.player_id = player_id;
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    packet.angle = angle;
    return packet;
}

// ============================================================================
// PACKET → COMPONENT CONVERTERS
// ============================================================================

/**
 * @brief Apply a SpawnEntity packet to the ECS registry
 *
 * Creates a new entity and adds Position and Velocity components.
 * The entity ID from the packet is used directly.
 */
inline void ApplySpawnEntityPacket(
    Engine::registry &reg, const network::SpawnEntityPacket &packet) {
    // Create entity with the network-provided ID
    auto entity = reg.entity_from_index(packet.entity_id);

    // Add Position component
    Component::Transform pos;
    pos.x = packet.pos_x;
    pos.y = packet.pos_y;
    reg.add_component(entity, std::move(pos));

    // Add Velocity component
    Component::Velocity vel;
    vel.vx = packet.vel_x;
    vel.vy = packet.vel_y;
    reg.add_component(entity, std::move(vel));
}

/**
 * @brief Update an existing entity's position from a packet
 */
inline void UpdatePositionFromPacket(Engine::registry &reg,
    network::EntityId entity_id, float pos_x, float pos_y) {
    auto entity = reg.entity_from_index(entity_id);
    auto &positions = reg.GetComponents<Component::Transform>();

    if (positions.has(entity.getId())) {
        positions[entity.getId()]->x = pos_x;
        positions[entity.getId()]->y = pos_y;
    }
}

/**
 * @brief Extract position and velocity from registry for an entity
 *
 * Returns nullopt if components don't exist.
 */
inline std::optional<std::pair<Component::Transform, Component::Velocity>>
GetEntityTransform(Engine::registry &reg, network::EntityId entity_id) {
    auto entity = reg.entity_from_index(entity_id);
    auto &positions = reg.GetComponents<Component::Transform>();
    auto &velocities = reg.GetComponents<Component::Velocity>();

    if (positions.has(entity.getId()) && velocities.has(entity.getId())) {
        return std::make_pair(
            *positions[entity.getId()], *velocities[entity.getId()]);
    }

    return std::nullopt;
}

/**
 * @brief Batch converter: Create spawn packets for all entities with Position
 *
 * Useful for sending initial game state to newly connected clients.
 */
inline std::vector<network::SpawnEntityPacket> CreateSnapshotPackets(
    Engine::registry &reg, network::Tick current_tick) {
    std::vector<network::SpawnEntityPacket> packets;

    auto &positions = reg.GetComponents<Component::Transform>();
    auto &velocities = reg.GetComponents<Component::Velocity>();

    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions.has(i) && velocities.has(i)) {
            auto packet = ToSpawnEntityPacket(current_tick,
                network::EntityId{static_cast<uint32_t>(i)},
                0,  // Default entity type, could be enhanced
                *positions[i], *velocities[i]);
            packets.push_back(packet);
        }
    }

    return packets;
}

}  // namespace server::converters
