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
 * Payload: 4 bytes (PlayerId u8 + Status u8 + UdpPort u16)
 */
struct ConnectAckPacket {
    static constexpr PacketType type = PacketType::ConnectAck;
    static constexpr size_t PAYLOAD_SIZE = 4;

    PlayerId player_id;
    uint8_t status;     // 0=OK, 1=ServerFull, 2=BadUsername, 3=InGame
    uint16_t udp_port;  // Server's UDP port (for client to send to)

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
        buffer.WriteUint16(udp_port);
    }

    static ConnectAckPacket Deserialize(PacketBuffer &buffer) {
        ConnectAckPacket packet;
        packet.player_id = PlayerId{buffer.ReadUint8()};
        packet.status = buffer.ReadUint8();
        packet.udp_port = buffer.ReadUint16();
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
    EntityId entity_id;   // 4 bytes
    uint8_t entity_type;  // 1 byte (sprite/prefab ID)
    uint8_t reserved;     // 1 byte padding
    uint16_t pos_x;       // 2 bytes (normalized 0..65535)
    uint16_t pos_y;       // 2 bytes (normalized 0..38864)
    uint16_t angle;       // 2 bytes (degrees 0..360)

    void Serialize(PacketBuffer &buffer) const {
        buffer.WriteUint32(entity_id.value);
        buffer.WriteUint8(entity_type);
        buffer.WriteUint8(reserved);
        buffer.WriteUint16(pos_x);
        buffer.WriteUint16(pos_y);
        buffer.WriteUint16(angle);
    }

    static EntityState Deserialize(PacketBuffer &buffer) {
        EntityState state;
        state.entity_id = EntityId{buffer.ReadUint32()};
        state.entity_type = buffer.ReadUint8();
        state.reserved = buffer.ReadUint8();
        state.pos_x = buffer.ReadUint16();
        state.pos_y = buffer.ReadUint16();
        state.angle = buffer.ReadUint16();
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
            entity.Serialize(buffer);
        }
    }

    static WorldSnapshotPacket Deserialize(PacketBuffer &buffer) {
        WorldSnapshotPacket packet;
        packet.entity_count = buffer.ReadUint16();
        packet.reserved[0] = buffer.ReadUint8();
        packet.reserved[1] = buffer.ReadUint8();
        packet.entities.reserve(packet.entity_count);
        for (uint16_t i = 0; i < packet.entity_count; ++i) {
            packet.entities.push_back(EntityState::Deserialize(buffer));
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
