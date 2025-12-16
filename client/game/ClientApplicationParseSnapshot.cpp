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

static uint32_t Get4BytesAsUint32(
    const std::array<uint8_t, 1460> &payload, size_t offset) {
    return static_cast<uint16_t>(payload[offset]) |
           (static_cast<uint16_t>(payload[offset + 1]) << 8) |
           (static_cast<uint16_t>(payload[offset + 2]) << 16) |
           (static_cast<uint16_t>(payload[offset + 3]) << 24);
}

static uint16_t Get2BytesAsUint16(
    const std::array<uint8_t, 1460> &payload, size_t offset) {
    return static_cast<uint16_t>(payload[offset]) |
           (static_cast<uint16_t>(payload[offset + 1]) << 8);
}

static uint8_t Get1ByteAsUint8(
    const std::array<uint8_t, 1460> &payload, size_t offset) {
    return static_cast<uint8_t>(payload[offset]);
}

// EntityState structure:
// - entity_id (u32, 4 bytes)
// - entity_type (u8, 1 byte)
// - reserved (u8, 1 byte)
// - pos_x (u16, 2 bytes)
// - pos_y (u16, 2 bytes)
// - angle (u16, 2 bytes)
// - velocity_x (u16, 2 bytes)
// - velocity_y (u16, 2 bytes)
void ParseSnapshotPlayer(
    std::vector<ClientApplication::ParsedEntity> &entities,
    const client::SnapshotPacket &snapshot) {
    const size_t kEntityStateSize =
        18;  // 18 bytes per EntityState (with health)

    // Check if payload contains WorldSnapshotPacket format (header + entities)
    // or just a single EntityState (current server implementation)
    if (snapshot.payload_size == kEntityStateSize) {
        // Single EntityState format (18 bytes) - current server implementation
        ClientApplication::ParsedEntity entity;

        entity.entity_id = Get4BytesAsUint32(snapshot.payload, 0);
        entity.entity_type = Get1ByteAsUint8(snapshot.payload, 4);
        // snapshot.payload[5] is reserved byte, skip it
        entity.pos_x = Get2BytesAsUint16(snapshot.payload, 6);
        entity.pos_y = Get2BytesAsUint16(snapshot.payload, 8);
        entity.angle = Get2BytesAsUint16(snapshot.payload, 10);
        // Decode velocity: encoded = (actual + 32768); decode:
        // (int16_t)encoded - 32768
        uint16_t encoded_vx = Get2BytesAsUint16(snapshot.payload, 12);
        uint16_t encoded_vy = Get2BytesAsUint16(snapshot.payload, 14);
        int16_t decoded_vx = static_cast<int16_t>(encoded_vx) - 32768;
        int16_t decoded_vy = static_cast<int16_t>(encoded_vy) - 32768;
        entity.velocity_x =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vx) + 32768);
        entity.velocity_y =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vy) + 32768);
        entity.health = Get2BytesAsUint16(snapshot.payload, 16);

        entities.push_back(entity);
    } else if (snapshot.payload_size >= 4) {
        // WorldSnapshotPacket format (with header)
        uint16_t entity_count = Get2BytesAsUint16(snapshot.payload, 0);

        size_t offset = 4;  // Skip header (entity_count + 2 reserved)

        // Parse each entity (12 bytes each)
        for (uint16_t i = 0; i < entity_count && offset + kEntityStateSize <=
                                                     snapshot.payload_size;
            ++i) {
            ClientApplication::ParsedEntity entity;

            entity.entity_id = Get4BytesAsUint32(snapshot.payload, offset + 0);
            entity.entity_type = Get1ByteAsUint8(snapshot.payload, offset + 4);
            // offset + 5 is reserved byte, skip it
            entity.pos_x = Get2BytesAsUint16(snapshot.payload, offset + 6);
            entity.pos_y = Get2BytesAsUint16(snapshot.payload, offset + 8);
            entity.angle = Get2BytesAsUint16(snapshot.payload, offset + 10);
            entity.velocity_x =
                Get2BytesAsUint16(snapshot.payload, offset + 12);
            entity.velocity_y =
                Get2BytesAsUint16(snapshot.payload, offset + 14);
            entity.health = Get2BytesAsUint16(snapshot.payload, offset + 16);

            entities.push_back(entity);
            offset += kEntityStateSize;
        }
    }
}

// EntityState structure:
// - entity_id (u32, 4 bytes)
// - entity_type (u8, 1 byte)
// - reserved (u8, 1 byte)
// - pos_x (u16, 2 bytes)
// - pos_y (u16, 2 bytes)
// - angle (u16, 2 bytes)
// - velocity_x (u16, 2 bytes)
// - velocity_y (u16, 2 bytes)
void ParseSnapshotEnemy(std::vector<ClientApplication::ParsedEntity> &entities,
    const client::SnapshotPacket &snapshot) {
    const size_t kEntityStateSize =
        20;  // 20 bytes per EntityState (with animation + health)

    // Check if payload contains WorldSnapshotPacket format (header + entities)
    // or just a single EntityState (current server implementation)
    if (snapshot.payload_size == kEntityStateSize) {
        // Single EntityState format (20 bytes) - current server implementation
        ClientApplication::ParsedEntity entity;

        entity.entity_id = Get4BytesAsUint32(snapshot.payload, 0);
        entity.entity_type = Get1ByteAsUint8(snapshot.payload, 4);
        // snapshot.payload[5] is reserved byte, skip it
        entity.pos_x = Get2BytesAsUint16(snapshot.payload, 6);
        entity.pos_y = Get2BytesAsUint16(snapshot.payload, 8);
        entity.angle = Get2BytesAsUint16(snapshot.payload, 10);
        // Decode velocity: encoded = (actual + 32768); decode:
        // (int16_t)encoded - 32768
        uint16_t encoded_vx = Get2BytesAsUint16(snapshot.payload, 12);
        uint16_t encoded_vy = Get2BytesAsUint16(snapshot.payload, 14);
        int16_t decoded_vx = static_cast<int16_t>(encoded_vx) - 32768;
        int16_t decoded_vy = static_cast<int16_t>(encoded_vy) - 32768;
        entity.velocity_x =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vx) + 32768);
        entity.velocity_y =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vy) + 32768);
        entity.current_animation = Get1ByteAsUint8(snapshot.payload, 16);
        entity.current_frame = Get1ByteAsUint8(snapshot.payload, 17);
        entity.health = Get2BytesAsUint16(snapshot.payload, 18);

        entities.push_back(entity);
    } else if (snapshot.payload_size >= 4) {
        // WorldSnapshotPacket format (with header)
        uint16_t entity_count = Get2BytesAsUint16(snapshot.payload, 0);

        size_t offset = 4;  // Skip header (entity_count + 2 reserved)

        // Parse each entity (12 bytes each)
        for (uint16_t i = 0; i < entity_count && offset + kEntityStateSize <=
                                                     snapshot.payload_size;
            ++i) {
            ClientApplication::ParsedEntity entity;

            entity.entity_id = Get4BytesAsUint32(snapshot.payload, offset + 0);
            entity.entity_type = Get1ByteAsUint8(snapshot.payload, offset + 4);
            // offset + 5 is reserved byte, skip it
            entity.pos_x = Get2BytesAsUint16(snapshot.payload, offset + 6);
            entity.pos_y = Get2BytesAsUint16(snapshot.payload, offset + 8);
            entity.angle = Get2BytesAsUint16(snapshot.payload, offset + 10);
            entity.velocity_x =
                Get2BytesAsUint16(snapshot.payload, offset + 12);
            entity.velocity_y =
                Get2BytesAsUint16(snapshot.payload, offset + 14);
            entity.current_animation =
                Get1ByteAsUint8(snapshot.payload, offset + 16);
            entity.current_frame =
                Get1ByteAsUint8(snapshot.payload, offset + 17);
            entity.health = Get2BytesAsUint16(snapshot.payload, offset + 18);

            entities.push_back(entity);
            offset += kEntityStateSize;
        }
    }
}

// EntityState structure:
// - entity_id (u32, 4 bytes)
// - entity_type (u8, 1 byte)
// - reserved (u8, 1 byte)
// - pos_x (u16, 2 bytes)
// - pos_y (u16, 2 bytes)
// - angle (u16, 2 bytes)
// - projectile_type (u8, 1 byte)
void ParseSnapshotProjectile(
    std::vector<ClientApplication::ParsedEntity> &entities,
    const client::SnapshotPacket &snapshot) {
    const size_t kEntityStateSize = 17;  // 13 bytes per EntityState

    // Check if payload contains WorldSnapshotPacket format (header + entities)
    // or just a single EntityState (current server implementation)
    if (snapshot.payload_size == kEntityStateSize) {
        // Single EntityState format (13 bytes) - current server implementation
        ClientApplication::ParsedEntity entity;

        entity.entity_id = Get4BytesAsUint32(snapshot.payload, 0);
        entity.entity_type = Get1ByteAsUint8(snapshot.payload, 4);
        // snapshot.payload[5] is reserved byte, skip it
        entity.pos_x = Get2BytesAsUint16(snapshot.payload, 6);
        entity.pos_y = Get2BytesAsUint16(snapshot.payload, 8);
        entity.angle = Get2BytesAsUint16(snapshot.payload, 10);
        // Decode velocity: encoded = (actual + 32768); decode:
        // (int16_t)encoded - 32768
        uint16_t encoded_vx = Get2BytesAsUint16(snapshot.payload, 12);
        uint16_t encoded_vy = Get2BytesAsUint16(snapshot.payload, 14);
        int16_t decoded_vx = static_cast<int16_t>(encoded_vx) - 32768;
        int16_t decoded_vy = static_cast<int16_t>(encoded_vy) - 32768;
        entity.velocity_x =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vx) + 32768);
        entity.velocity_y =
            static_cast<uint16_t>(static_cast<int32_t>(decoded_vy) + 32768);
        entity.projectile_type = Get1ByteAsUint8(snapshot.payload, 16);

        entities.push_back(entity);
    } else if (snapshot.payload_size >= 4) {
        // WorldSnapshotPacket format (with header)
        uint16_t entity_count = Get2BytesAsUint16(snapshot.payload, 0);

        size_t offset = 4;  // Skip header (entity_count + 2 reserved)

        // Parse each entity (12 bytes each)
        for (uint16_t i = 0; i < entity_count && offset + kEntityStateSize <=
                                                     snapshot.payload_size;
            ++i) {
            ClientApplication::ParsedEntity entity;

            entity.entity_id = Get4BytesAsUint32(snapshot.payload, offset + 0);
            entity.entity_type = Get1ByteAsUint8(snapshot.payload, offset + 4);
            // offset + 5 is reserved byte, skip it
            entity.pos_x = Get2BytesAsUint16(snapshot.payload, offset + 6);
            entity.pos_y = Get2BytesAsUint16(snapshot.payload, offset + 8);
            entity.angle = Get2BytesAsUint16(snapshot.payload, offset + 10);
            entity.velocity_x =
                Get2BytesAsUint16(snapshot.payload, offset + 12);
            entity.velocity_y =
                Get2BytesAsUint16(snapshot.payload, offset + 14);
            entity.projectile_type =
                Get1ByteAsUint8(snapshot.payload, offset + 16);

            entities.push_back(entity);
            offset += kEntityStateSize;
        }
    }
}

/**
 * @brief Parse snapshot data from UDP packet.
 *
 * Deserializes EntityState structures from the snapshot payload.
 * Current implementation: Server sends a single EntityState directly (12
 * bytes) without WorldSnapshotPacket header.
 *
 * @param snapshot The snapshot packet received from server
 * @return Vector of parsed entities
 */
std::vector<ClientApplication::ParsedEntity>
ClientApplication::ParseSnapshotData(const client::SnapshotPacket &snapshot) {
    std::vector<ClientApplication::ParsedEntity> entities;
    if (snapshot.entity_type == 0x00) {
        // Parse using the existing function
        ParseSnapshotPlayer(entities, snapshot);
    } else if (snapshot.entity_type == 0x01) {
        // Parse using the enemy parsing function
        ParseSnapshotEnemy(entities, snapshot);
    } else if (snapshot.entity_type == 0x02) {
        // Similar parsing function can be created for projectiles if needed
        // For now, we can reuse the same parsing logic
        ParseSnapshotProjectile(entities, snapshot);
    } else {
        // Unknown entity type, fallback to player parsing
    }
    return entities;
}

/**
 * @brief Display snapshot data (for debugging).
 *
 * @param snapshot The snapshot packet received from server
 */
void ClientApplication::DisplaySnapshotData(
    const client::SnapshotPacket &snapshot) {
    static int last_tick = -1;
    static int display_count = 0;

    display_count++;

    std::cout << "\n[UDP Snapshot] Tick=" << snapshot.tick
              << " PayloadSize=" << snapshot.payload_size << " bytes"
              << std::endl;

    // Display raw payload bytes (first 12 bytes)
    if (display_count <= 3 && snapshot.payload_size > 0) {
        std::cout << "  Raw payload (hex): ";
        for (size_t i = 0; i < std::min<size_t>(snapshot.payload_size, 12);
            ++i) {
            std::cout << std::hex << std::setfill('0') << std::setw(2)
                      << static_cast<int>(snapshot.payload[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    // Parse entities using the new function
    auto entities = ParseSnapshotData(snapshot);

    std::cout << "  Entities: " << entities.size() << std::endl;

    for (size_t i = 0; i < entities.size(); ++i) {
        const auto &entity = entities[i];
        std::cout << "    [" << i << "] ID=" << entity.entity_id << " Type=0x"
                  << std::hex << static_cast<int>(entity.entity_type)
                  << std::dec << " Pos=(" << entity.pos_x << ","
                  << entity.pos_y << ") Angle=" << entity.angle << " Vel=("
                  << entity.velocity_x << "," << entity.velocity_y << ")"
                  << std::endl;
    }
    std::cout << std::flush;
}

}  // namespace Rtype::Client
