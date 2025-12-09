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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        for (char c : username) {
            buffer.write_uint8(static_cast<uint8_t>(c));
        }
    }

    static ConnectReqPacket deserialize(PacketBuffer &buffer) {
        ConnectReqPacket packet;
        for (size_t i = 0; i < 32; ++i) {
            packet.username[i] = static_cast<char>(buffer.read_uint8());
        }
        return packet;
    }

    void set_username(const std::string &name) {
        username.fill('\0');
        size_t len = std::min(name.size(), size_t(31));
        std::copy_n(name.begin(), len, username.begin());
    }

    std::string get_username() const {
        return std::string(username.data());
    }
};

/**
 * @brief TCP 0x02: CONNECT_ACK - Server responds to login
 * RFC Section 5.2
 * Payload: 4 bytes (PlayerId u8 + Status u8 + Reserved u8[2])
 */
struct ConnectAckPacket {
    static constexpr PacketType type = PacketType::ConnectAck;
    static constexpr size_t PAYLOAD_SIZE = 4;

    PlayerId player_id;
    uint8_t status;  // 0=OK, 1=ServerFull, 2=BadUsername, 3=InGame
    std::array<uint8_t, 2> reserved;

    enum Status : uint8_t {
        OK = 0,
        ServerFull = 1,
        BadUsername = 2,
        InGame = 3
    };

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        buffer.write_uint8(player_id.value);
        buffer.write_uint8(status);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
    }

    static ConnectAckPacket deserialize(PacketBuffer &buffer) {
        ConnectAckPacket packet;
        packet.player_id = PlayerId{buffer.read_uint8()};
        packet.status = buffer.read_uint8();
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
    }

    static DisconnectReqPacket deserialize(PacketBuffer &buffer) {
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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        buffer.write_uint8(player_id.value);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        buffer.write_uint8(reserved[2]);
    }

    static NotifyDisconnectPacket deserialize(PacketBuffer &buffer) {
        NotifyDisconnectPacket packet;
        packet.player_id = PlayerId{buffer.read_uint8()};
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
        packet.reserved[2] = buffer.read_uint8();
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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        buffer.write_uint32(controlled_entity_id.value);
    }

    static GameStartPacket deserialize(PacketBuffer &buffer) {
        GameStartPacket packet;
        packet.controlled_entity_id = EntityId{buffer.read_uint32()};
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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        buffer.write_uint8(winning_player_id.value);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        buffer.write_uint8(reserved[2]);
    }

    static GameEndPacket deserialize(PacketBuffer &buffer) {
        GameEndPacket packet;
        packet.winning_player_id = PlayerId{buffer.read_uint8()};
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
        packet.reserved[2] = buffer.read_uint8();
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

    CommonHeader make_header() const {
        return CommonHeader(static_cast<uint8_t>(type), PAYLOAD_SIZE);
    }

    void serialize(PacketBuffer &buffer) const {
        buffer.write_header(make_header());
        buffer.write_uint8(is_ready);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        buffer.write_uint8(reserved[2]);
    }

    static ReadyStatusPacket deserialize(PacketBuffer &buffer) {
        ReadyStatusPacket packet;
        packet.is_ready = buffer.read_uint8();
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
        packet.reserved[2] = buffer.read_uint8();
        return packet;
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

    CommonHeader make_header(uint32_t tick_id) const {
        return CommonHeader(
            static_cast<uint8_t>(type), PAYLOAD_SIZE, tick_id, 0, 1);
    }

    void serialize(PacketBuffer &buffer, uint32_t tick_id) const {
        buffer.write_header(make_header(tick_id));
        buffer.write_uint8(inputs.value);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        buffer.write_uint8(reserved[2]);
    }

    static PlayerInputPacket deserialize(PacketBuffer &buffer) {
        PlayerInputPacket packet;
        packet.inputs = InputFlags{buffer.read_uint8()};
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
        packet.reserved[2] = buffer.read_uint8();
        return packet;
    }
};

/**
 * @brief Entity state for WORLD_SNAPSHOT
 * RFC Section 6.2: 12 bytes per entity (aligned)
 */
struct EntityState {
    EntityId entity_id;   // 4 bytes
    uint8_t entity_type;  // 1 byte (sprite/prefab ID)
    uint8_t reserved;     // 1 byte padding
    uint16_t pos_x;       // 2 bytes (normalized 0..65535)
    uint16_t pos_y;       // 2 bytes (normalized 0..38864)
    uint16_t angle;       // 2 bytes (degrees 0..360)

    void serialize(PacketBuffer &buffer) const {
        buffer.write_uint32(entity_id.value);
        buffer.write_uint8(entity_type);
        buffer.write_uint8(reserved);
        buffer.write_uint16(pos_x);
        buffer.write_uint16(pos_y);
        buffer.write_uint16(angle);
    }

    static EntityState deserialize(PacketBuffer &buffer) {
        EntityState state;
        state.entity_id = EntityId{buffer.read_uint32()};
        state.entity_type = buffer.read_uint8();
        state.reserved = buffer.read_uint8();
        state.pos_x = buffer.read_uint16();
        state.pos_y = buffer.read_uint16();
        state.angle = buffer.read_uint16();
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

    size_t payload_size() const {
        return 4 + (entities.size() * 12);
    }

    CommonHeader make_header(
        uint32_t tick_id, uint8_t packet_index, uint8_t packet_count) const {
        return CommonHeader(static_cast<uint8_t>(type),
            static_cast<uint16_t>(payload_size()), tick_id, packet_index,
            packet_count);
    }

    void serialize(PacketBuffer &buffer, uint32_t tick_id,
        uint8_t packet_index = 0, uint8_t packet_count = 1) const {
        buffer.write_header(make_header(tick_id, packet_index, packet_count));
        buffer.write_uint16(entity_count);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        for (const auto &entity : entities) {
            entity.serialize(buffer);
        }
    }

    static WorldSnapshotPacket deserialize(PacketBuffer &buffer) {
        WorldSnapshotPacket packet;
        packet.entity_count = buffer.read_uint16();
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();

        packet.entities.reserve(packet.entity_count);
        for (uint16_t i = 0; i < packet.entity_count; ++i) {
            packet.entities.push_back(EntityState::deserialize(buffer));
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

    CommonHeader make_header(uint32_t tick_id) const {
        return CommonHeader(
            static_cast<uint8_t>(type), PAYLOAD_SIZE, tick_id, 0, 1);
    }

    void serialize(PacketBuffer &buffer, uint32_t tick_id) const {
        buffer.write_header(make_header(tick_id));
        buffer.write_uint8(player_id.value);
        buffer.write_uint8(lives);
        buffer.write_uint8(reserved[0]);
        buffer.write_uint8(reserved[1]);
        buffer.write_uint32(score);
    }

    static PlayerStatsPacket deserialize(PacketBuffer &buffer) {
        PlayerStatsPacket packet;
        packet.player_id = PlayerId{buffer.read_uint8()};
        packet.lives = buffer.read_uint8();
        packet.reserved[0] = buffer.read_uint8();
        packet.reserved[1] = buffer.read_uint8();
        packet.score = buffer.read_uint32();
        return packet;
    }
};

}  // namespace server::network
