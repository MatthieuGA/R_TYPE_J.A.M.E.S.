#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "server/PacketBuffer.hpp"
#include "server/PacketTypes.hpp"

namespace server::network {

// ============================================================================
// TCP PACKETS (Session Management - RFC Section 5)
// ============================================================================

/**
 * @brief TCP 0x01: CONNECT_REQ - Client requests to join lobby
 * RFC Section 5.1
 * Payload: 32 bytes (username)
 */
struct ConnectReqPacket {
    static constexpr PacketType type = PacketType::ConnectReq;
    static constexpr size_t PAYLOAD_SIZE = 32;

    std::array<char, 32> username;  // Null-terminated, fixed size

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        for (char c : username) {
            buffer.WriteUint8(static_cast<uint8_t>(c));
        }
    }

    static ConnectReqPacket Deserialize(PacketBuffer &buffer) {
        ConnectReqPacket packet;
        for (size_t i = 0; i < 32; ++i) {
            packet.username[i] = static_cast<char>(buffer.ReadUint8());
        }
        return packet;
    }

    void SetUsername(const std::string &name) {
        username.fill('\0');
        size_t len = std::min(name.size(), size_t(31));
        std::copy_n(name.begin(), len, username.begin());
    }

    std::string GetUsername() const {
        return std::string(username.data());
    }
};

/**
 * @brief TCP 0x02: CONNECT_ACK - Server responds to login
 * RFC Section 5.2
 * Payload: 8 bytes (PlayerId u8 + Status u8 + ConnectedPlayers u8 +
 *                   ReadyPlayers u8 + MaxPlayers u8 + MinPlayers u8 + Reserved
 * u16)
 */
struct ConnectAckPacket {
    static constexpr PacketType type = PacketType::ConnectAck;
    static constexpr size_t PAYLOAD_SIZE = 8;

    PlayerId player_id;
    uint8_t status;             // 0=OK, 1=ServerFull, 2=BadUsername, 3=InGame
    uint8_t connected_players;  // Number of currently connected players
    uint8_t ready_players;      // Number of ready players
    uint8_t max_players;        // Maximum players allowed
    uint8_t min_players;        // Minimum players to start
    uint16_t reserved;          // Reserved for alignment

    enum Status : uint8_t {
        OK = 0,
        ServerFull = 1,
        BadUsername = 2,
        InGame = 3
    };

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(player_id.value);
        buffer.WriteUint8(status);
        buffer.WriteUint8(connected_players);
        buffer.WriteUint8(ready_players);
        buffer.WriteUint8(max_players);
        buffer.WriteUint8(min_players);
        buffer.WriteUint16(reserved);
    }

    static ConnectAckPacket Deserialize(PacketBuffer &buffer) {
        ConnectAckPacket packet;
        packet.player_id = PlayerId{buffer.ReadUint8()};
        packet.status = buffer.ReadUint8();
        packet.connected_players = buffer.ReadUint8();
        packet.ready_players = buffer.ReadUint8();
        packet.max_players = buffer.ReadUint8();
        packet.min_players = buffer.ReadUint8();
        packet.reserved = buffer.ReadUint16();
        return packet;
    }
};

/**
 * @brief TCP 0x03: DISCONNECT_REQ - Client requests to leave
 * RFC Section 5.3
 * Payload: 0 bytes (header only)
 */
struct DisconnectReqPacket {
    static constexpr PacketType type = PacketType::DisconnectReq;
    static constexpr size_t PAYLOAD_SIZE = 0;

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
    }

    static DisconnectReqPacket Deserialize(PacketBuffer &buffer) {
        (void)buffer;  // No payload to read
        return DisconnectReqPacket{};
    }
};

/**
 * @brief TCP 0x04: NOTIFY_DISCONNECT - Server notifies of player disconnect
 * RFC Section 5.4
 * Payload: 4 bytes (PlayerId u8 + Reserved u8[3])
 */
struct NotifyDisconnectPacket {
    static constexpr PacketType type = PacketType::NotifyDisconnect;
    static constexpr size_t PAYLOAD_SIZE = 4;

    PlayerId player_id;
    std::array<uint8_t, 3> reserved;

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(player_id.value);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint8(reserved[2]);
    }

    static NotifyDisconnectPacket Deserialize(PacketBuffer &buffer) {
        NotifyDisconnectPacket packet;
        packet.player_id = PlayerId{buffer.ReadUint8()};
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.reserved[2] = buffer.ReadUint8();
        return packet;
    }
};

/**
 * @brief TCP 0x05: GAME_START - Server triggers match start
 * RFC Section 5.5
 * Payload: 4 bytes (ControlledEntityId u32)
 */
struct GameStartPacket {
    static constexpr PacketType type = PacketType::GameStart;
    static constexpr size_t PAYLOAD_SIZE = 4;

    EntityId controlled_entity_id;

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint32(controlled_entity_id.value);
    }

    static GameStartPacket Deserialize(PacketBuffer &buffer) {
        GameStartPacket packet;
        packet.controlled_entity_id = EntityId{buffer.ReadUint32()};
        return packet;
    }
};

/**
 * @brief TCP 0x06: GAME_END - Server announces match end
 * RFC Section 5.6
 * Payload: 4 bytes (WinningPlayerId u8 + Reserved u8[3])
 */
struct GameEndPacket {
    static constexpr PacketType type = PacketType::GameEnd;
    static constexpr size_t PAYLOAD_SIZE = 4;

    PlayerId winning_player_id;  // 0 = draw
    std::array<uint8_t, 3> reserved;

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(winning_player_id.value);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint8(reserved[2]);
    }

    static GameEndPacket Deserialize(PacketBuffer &buffer) {
        GameEndPacket packet;
        packet.winning_player_id = PlayerId{buffer.ReadUint8()};
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.reserved[2] = buffer.ReadUint8();
        return packet;
    }
};

/**
 * @brief TCP 0x0A: SET_GAME_SPEED - Client sets game speed multiplier
 * Payload: 4 bytes (float speed multiplier, range 0.25-2.0)
 */
struct SetGameSpeedPacket {
    static constexpr PacketType type = PacketType::SetGameSpeed;
    static constexpr size_t PAYLOAD_SIZE = 4;

    float speed;  // Game speed multiplier (0.25 = 25%, 2.0 = 200%)

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteFloat(speed);
    }

    static SetGameSpeedPacket Deserialize(PacketBuffer &buffer) {
        SetGameSpeedPacket packet;
        packet.speed = buffer.ReadFloat();
        return packet;
    }
};

/**
 * @brief TCP 0x07: READY_STATUS - Client indicates ready state
 * RFC Section 5.7
 * Payload: 1 byte (IsReady u8) + 3 bytes reserved (aligned to 4)
 */
struct ReadyStatusPacket {
    static constexpr PacketType type = PacketType::ReadyStatus;
    static constexpr size_t PAYLOAD_SIZE = 4;

    uint8_t is_ready;  // 0=Not Ready, 1=Ready
    std::array<uint8_t, 3> reserved;

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(is_ready);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint8(reserved[2]);
    }

    static ReadyStatusPacket Deserialize(PacketBuffer &buffer) {
        ReadyStatusPacket packet;
        packet.is_ready = buffer.ReadUint8();
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.reserved[2] = buffer.ReadUint8();
        return packet;
    }
};

/**
 * @brief TCP 0x08: NOTIFY_CONNECT - Server broadcasts new player connection
 * RFC Section 5.8
 * Payload: 36 bytes (PlayerId u8 + Reserved u8[3] + Username char[32])
 */
struct NotifyConnectPacket {
    static constexpr PacketType type = PacketType::NotifyConnect;
    static constexpr size_t PAYLOAD_SIZE = 36;

    PlayerId player_id;
    std::array<uint8_t, 3> reserved;
    std::array<char, 32> username;

    NotifyConnectPacket() : player_id(0), reserved{0, 0, 0}, username{} {}

    NotifyConnectPacket(PlayerId pid, const std::string &uname)
        : player_id(pid), reserved{0, 0, 0}, username{} {
        size_t len = std::min(uname.size(), username.size() - 1);
        std::copy(uname.begin(), uname.begin() + len, username.begin());
        username[len] = '\0';
    }

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(player_id.value);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint8(reserved[2]);
        for (size_t i = 0; i < username.size(); ++i) {
            buffer.WriteUint8(static_cast<uint8_t>(username[i]));
        }
    }

    static NotifyConnectPacket Deserialize(PacketBuffer &buffer) {
        NotifyConnectPacket pkt;
        pkt.player_id = PlayerId{buffer.ReadUint8()};
        pkt.reserved[0] = buffer.ReadUint8();
        pkt.reserved[1] = buffer.ReadUint8();
        pkt.reserved[2] = buffer.ReadUint8();
        for (size_t i = 0; i < pkt.username.size(); ++i) {
            pkt.username[i] = static_cast<char>(buffer.ReadUint8());
        }
        return pkt;
    }
};

/**
 * @brief TCP 0x09: NOTIFY_READY - Server broadcasts player ready status change
 * RFC Section 5.9
 * Payload: 4 bytes (PlayerId u8 + IsReady u8 + Reserved u8[2])
 */
struct NotifyReadyPacket {
    static constexpr PacketType type = PacketType::NotifyReady;
    static constexpr size_t PAYLOAD_SIZE = 4;

    PlayerId player_id;
    uint8_t is_ready;
    std::array<uint8_t, 2> reserved;

    NotifyReadyPacket() : player_id(0), is_ready(0), reserved{0, 0} {}

    NotifyReadyPacket(PlayerId pid, bool ready)
        : player_id(pid), is_ready(ready ? 1 : 0), reserved{0, 0} {}

    CommonHeader MakeHeader() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteHeader(MakeHeader());
        buffer.WriteUint8(player_id.value);
        buffer.WriteUint8(is_ready);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
    }

    static NotifyReadyPacket Deserialize(PacketBuffer &buffer) {
        NotifyReadyPacket pkt;
        pkt.player_id = PlayerId{buffer.ReadUint8()};
        pkt.is_ready = buffer.ReadUint8();
        pkt.reserved[0] = buffer.ReadUint8();
        pkt.reserved[1] = buffer.ReadUint8();
        return pkt;
    }
};

// ============================================================================
// UDP PACKETS (Real-time Gameplay - RFC Section 6)
// ============================================================================

/**
 * @brief UDP 0x10: PLAYER_INPUT - Client sends input bitmask
 * RFC Section 6.1
 * Payload: 4 bytes (Inputs u8 + Reserved u8[3])
 */
struct PlayerInputPacket {
    static constexpr PacketType type = PacketType::PlayerInput;
    static constexpr size_t PAYLOAD_SIZE = 4;

    InputFlags inputs;
    std::array<uint8_t, 3> reserved;

    CommonHeader MakeHeader(uint32_t tick_id) const {
        return CommonHeader(
            static_cast<uint8_t>(type), PAYLOAD_SIZE, tick_id, 0, 1);
    }

    void Serialize(PacketBuffer &buffer, uint32_t tick_id) const {
        buffer.WriteHeader(MakeHeader(tick_id));
        buffer.WriteUint8(inputs.value);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint8(reserved[2]);
    }

    static PlayerInputPacket Deserialize(PacketBuffer &buffer) {
        PlayerInputPacket packet;
        packet.inputs = InputFlags{buffer.ReadUint8()};
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.reserved[2] = buffer.ReadUint8();
        return packet;
    }
};

/**
 * @brief Entity state for WORLD_SNAPSHOT
 * RFC Section 6.2: 12 bytes per entity (aligned)
 */
struct EntityState {
    enum class EntityType : uint8_t {
        Player = 0x00,
        Enemy = 0x01,
        Projectile = 0x02
    };

    enum class EnemyType : uint8_t {
        Mermaid = 0x00,
        KamiFish = 0x01,
        Daemon = 0x02,
        Golem = 0x03,
        Invinsibility = 0x04,
        Health = 0x05,
        Gatling = 0x06,
        ArchDemon = 0x07
    };

    EntityId entity_id;           // 4 bytes
    uint8_t entity_type;          // 1 byte (sprite/prefab ID)
    uint8_t reserved;             // 1 byte padding
    uint16_t pos_x;               // 2 bytes (normalized 0..65535)
    uint16_t pos_y;               // 2 bytes (normalized 0..38864)
    uint16_t angle;               // 2 bytes (degrees 0..360)
    uint16_t velocity_x;          // 2 bytes (normalized -32768..32767)
    uint16_t velocity_y;          // 2 bytes (normalized -32768..32767)
    uint8_t projectile_type;      // 1 byte (for projectiles)
    uint8_t enemy_type;           // 1 byte (for enemies)
    uint8_t current_animation;    // 1 byte (for animated entities)
    uint8_t current_frame;        // 1 byte (for animated entities)
    uint16_t health;              // 2 bytes (health or hit points)
    uint16_t invincibility_time;  // 2 bytes (invincibility time in ms)
    uint16_t score;               // 2 bytes (player score)

    void Serialize(PacketBuffer &buffer, EntityType type) const {
        buffer.WriteUint32(entity_id.value);  // 4 bytes
        buffer.WriteUint8(entity_type);       // 1 byte
        buffer.WriteUint8(reserved);          // 1 byte
        if (type == EntityType::Player) {
            buffer.WriteUint16(pos_x);               // 2 bytes
            buffer.WriteUint16(pos_y);               // 2 bytes
            buffer.WriteUint16(angle);               // 2 bytes
            buffer.WriteUint16(velocity_x);          // 2 bytes
            buffer.WriteUint16(velocity_y);          // 2 bytes
            buffer.WriteUint16(health);              // 2 byte
            buffer.WriteUint16(invincibility_time);  // 2 byte
            buffer.WriteUint16(score);               // 2 byte
        } else if (type == EntityType::Enemy) {
            buffer.WriteUint16(pos_x);             // 2 bytes
            buffer.WriteUint16(pos_y);             // 2 bytes
            buffer.WriteUint16(angle);             // 2 bytes
            buffer.WriteUint16(velocity_x);        // 2 bytes
            buffer.WriteUint16(velocity_y);        // 2 bytes
            buffer.WriteUint8(enemy_type);         // 1 byte
            buffer.WriteUint8(current_animation);  // 1 byte
            buffer.WriteUint8(current_frame);      // 1 byte
            buffer.WriteUint16(health);            // 2 byte
        } else if (type == EntityType::Projectile) {
            buffer.WriteUint16(pos_x);           // 2 bytes
            buffer.WriteUint16(pos_y);           // 2 bytes
            buffer.WriteUint16(angle);           // 2 bytes
            buffer.WriteUint16(velocity_x);      // 2 bytes
            buffer.WriteUint16(velocity_y);      // 2 bytes
            buffer.WriteUint8(projectile_type);  // 1 byte
        }
    }

    static EntityState Deserialize(
        PacketBuffer &buffer, EntityType type = EntityType::Player) {
        EntityState state;
        state.entity_id = EntityId{buffer.ReadUint32()};
        state.entity_type = buffer.ReadUint8();
        state.reserved = buffer.ReadUint8();
        if (type == EntityType::Player) {
            state.pos_x = buffer.ReadUint16();
            state.pos_y = buffer.ReadUint16();
            state.angle = buffer.ReadUint16();
            state.velocity_x = buffer.ReadUint16();
            state.velocity_y = buffer.ReadUint16();
            state.health = buffer.ReadUint16();
            state.invincibility_time = buffer.ReadUint16();
            state.score = buffer.ReadUint16();
        } else if (type == EntityType::Enemy) {
            state.pos_x = buffer.ReadUint16();
            state.pos_y = buffer.ReadUint16();
            state.angle = buffer.ReadUint16();
            state.velocity_x = buffer.ReadUint16();
            state.velocity_y = buffer.ReadUint16();
            state.enemy_type = buffer.ReadUint8();
            state.current_animation = buffer.ReadUint8();
            state.current_frame = buffer.ReadUint8();
            state.health = buffer.ReadUint16();
        } else if (type == EntityType::Projectile) {
            state.pos_x = buffer.ReadUint16();
            state.pos_y = buffer.ReadUint16();
            state.angle = buffer.ReadUint16();
            state.velocity_x = buffer.ReadUint16();
            state.velocity_y = buffer.ReadUint16();
            state.projectile_type = buffer.ReadUint8();
        }
        return state;
    }
};

/**
 * @brief UDP 0x20: WORLD_SNAPSHOT - Server broadcasts full game state
 * RFC Section 6.2
 * Payload: 4 bytes + (entity_count * 12 bytes)
 *
 * Note: May be fragmented across multiple packets if entity count is large
 */
struct WorldSnapshotPacket {
    static constexpr PacketType type = PacketType::WorldSnapshot;

    uint16_t entity_count;
    std::array<uint8_t, 2> reserved;
    std::vector<EntityState> entities;

    size_t PayloadSize() const {
        return 4 + (entities.size() * 12);
    }

    CommonHeader MakeHeader(
        uint32_t tick_id, uint8_t packet_index, uint8_t packet_count) const {
        return CommonHeader(static_cast<uint8_t>(type),
            static_cast<uint16_t>(PayloadSize()), tick_id, packet_index,
            packet_count);
    }

    void Serialize(PacketBuffer &buffer, uint32_t tick_id,
        uint8_t packet_index = 0, uint8_t packet_count = 1) const {
        buffer.WriteHeader(MakeHeader(tick_id, packet_index, packet_count));
        buffer.WriteUint16(entity_count);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        for (const auto &entity : entities) {
            entity.Serialize(buffer,
                static_cast<EntityState::EntityType>(entity.entity_type));
        }
    }

    static WorldSnapshotPacket Deserialize(PacketBuffer &buffer,
        EntityState::EntityType type = EntityState::EntityType::Player) {
        WorldSnapshotPacket packet;
        packet.entity_count = buffer.ReadUint16();
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.entities.reserve(packet.entity_count);
        for (uint16_t i = 0; i < packet.entity_count; ++i) {
            packet.entities.push_back(EntityState::Deserialize(buffer, type));
        }
        return packet;
    }
};

/**
 * @brief UDP 0x21: PLAYER_STATS - Server sends HUD updates
 * RFC Section 6.3
 * Payload: 8 bytes (PlayerId u8 + Lives u8 + Reserved u16 + Score u32)
 */
struct PlayerStatsPacket {
    static constexpr PacketType type = PacketType::PlayerStats;
    static constexpr size_t PAYLOAD_SIZE = 8;

    PlayerId player_id;
    uint8_t lives;
    std::array<uint8_t, 2> reserved;
    uint32_t score;

    CommonHeader MakeHeader(uint32_t tick_id) const {
        return CommonHeader(
            static_cast<uint8_t>(type), PAYLOAD_SIZE, tick_id, 0, 1);
    }

    void Serialize(PacketBuffer &buffer, uint32_t tick_id) const {
        buffer.WriteHeader(MakeHeader(tick_id));
        buffer.WriteUint8(player_id.value);
        buffer.WriteUint8(lives);
        buffer.WriteUint8(reserved[0]);
        buffer.WriteUint8(reserved[1]);
        buffer.WriteUint32(score);
    }

    static PlayerStatsPacket Deserialize(PacketBuffer &buffer) {
        PlayerStatsPacket packet;
        packet.player_id = PlayerId{buffer.ReadUint8()};
        packet.lives = buffer.ReadUint8();
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.score = buffer.ReadUint32();
        return packet;
    }
};

}  // namespace server::network
