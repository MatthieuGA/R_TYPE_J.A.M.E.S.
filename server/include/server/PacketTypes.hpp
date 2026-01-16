#pragma once

#include <cstdint>

namespace server::network {

/**
 * @brief RFC-compliant packet opcodes (Section 3.2)
 *
 * OpCode ranges:
 *   0x00-0x3F: TCP (Session Management)
 *   0x40-0x7F: UDP (Client Inputs)
 *   0x80-0xBF: UDP (Server Snapshots)
 */
enum class PacketType : uint8_t {
    // TCP Session Management (0x01-0x0B)
    ConnectReq = 0x01,        // Client -> Server: Login request
    ConnectAck = 0x02,        // Server -> Client: Login response
    DisconnectReq = 0x03,     // Client -> Server: Leave request
    NotifyDisconnect = 0x04,  // Server -> Client: Player left
    GameStart = 0x05,         // Server -> Client: Match begins
    GameEnd = 0x06,           // Server -> Client: Match ends
    ReadyStatus = 0x07,       // Client -> Server: Ready state
    NotifyConnect = 0x08,     // Server -> Client: New player joined
    NotifyReady = 0x09,       // Server -> Client: Player ready status changed
    SetGameSpeed = 0x0A,      // Client -> Server: Set game speed multiplier
    NotifyGameSpeed = 0x0B,   // Server -> Client: Game speed changed
    SetDifficulty = 0x0C,     // Client -> Server: Set difficulty level
    SetKillableProjectiles =
        0x0D,                 // Client -> Server: Set killable projectiles
    NotifyDifficulty = 0x0E,  // Server -> Client: Difficulty changed
    NotifyKillableProjectiles =
        0x0F,  // Server -> Client: Killable projectiles changed

    // UDP Client Inputs (0x10+)
    PlayerInput = 0x10,  // Client -> Server: Input bitmask

    // UDP Server Snapshots (0x20+)
    WorldSnapshot = 0x20,  // Server -> Client: Full game state
    PlayerStats = 0x21,    // Server -> Client: HUD updates
};

// Strong types for game identifiers
struct PlayerId {
    uint8_t value;  // RFC Section 3.1: u8 (max 255 players)

    constexpr PlayerId() : value(0) {}

    constexpr explicit PlayerId(uint8_t v) : value(v) {}

    constexpr bool operator==(const PlayerId &other) const {
        return value == other.value;
    }
};

struct EntityId {
    uint32_t value;

    constexpr EntityId() : value(0) {}

    constexpr explicit EntityId(uint32_t v) : value(v) {}

    constexpr bool operator==(const EntityId &other) const {
        return value == other.value;
    }
};

struct Tick {
    uint64_t value;

    constexpr Tick() : value(0) {}

    constexpr explicit Tick(uint64_t v) : value(v) {}

    constexpr bool operator==(const Tick &other) const {
        return value == other.value;
    }
};

struct InputFlags {
    uint8_t value;

    // RFC Section 6.1: Input bitmask
    static constexpr uint8_t UP = 0x01;
    static constexpr uint8_t DOWN = 0x02;
    static constexpr uint8_t LEFT = 0x04;
    static constexpr uint8_t RIGHT = 0x08;
    static constexpr uint8_t SHOOT = 0x10;

    constexpr InputFlags() : value(0) {}

    constexpr explicit InputFlags(uint8_t v) : value(v) {}

    constexpr bool has(uint8_t flag) const {
        return (value & flag) != 0;
    }

    constexpr void set(uint8_t flag) {
        value |= flag;
    }

    constexpr void clear(uint8_t flag) {
        value &= ~flag;
    }
};

}  // namespace server::network
