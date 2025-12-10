# RFC-Compliant Packet System Implementation

## Overview

This document describes the complete refactoring of the R-Type packet system to be fully compliant with the Network RFC (v3.2.0) specified in `docs/docs/network-rfc.txt`.

## Changes Summary

### 1. Header Structure (12 bytes, packed)

**Before:** Each packet wrote its own type as first 2 bytes (uint16)

**After:** All packets now use a standardized 12-byte CommonHeader (RFC Section 4.1):

```cpp
struct __attribute__((packed)) CommonHeader {
    uint8_t op_code;           // 1 byte: Command identifier
    uint16_t payload_size;     // 2 bytes: Payload size after header
    uint8_t packet_index;      // 1 byte: Fragment index
    uint32_t tick_id;          // 4 bytes: Frame counter (0 for TCP)
    uint8_t packet_count;      // 1 byte: Total fragments
    uint8_t reserved[3];       // 3 bytes: Padding (must be 0)
};
// Total: 12 bytes (verified with sizeof())
```

### 2. OpCode Changes

**Before:** `enum class PacketType : uint16_t` with values like `0x0001`, `0x0010`

**After:** `enum class PacketType : uint8_t` with RFC-compliant ranges:
- **TCP (0x01-0x07):** Session Management
  - `0x01` CONNECT_REQ
  - `0x02` CONNECT_ACK
  - `0x03` DISCONNECT_REQ
  - `0x04` NOTIFY_DISCONNECT
  - `0x05` GAME_START
  - `0x06` GAME_END
  - `0x07` READY_STATUS
- **UDP (0x10+):** Real-time Gameplay
  - `0x10` PLAYER_INPUT
  - `0x20` WORLD_SNAPSHOT
  - `0x21` PLAYER_STATS

### 3. PlayerId Type

**Before:** `uint32_t value` (4 bytes, max 4 billion players)

**After:** `uint8_t value` (1 byte, max 255 players per RFC Section 3.1)

### 4. Packet Structures

#### TCP Packets (All New)

All TCP packets follow this pattern:
1. 12-byte CommonHeader with `tick_id = 0`
2. Fixed-size payload (aligned to 4 bytes where appropriate)
3. Deserialize methods read payload only (header already consumed)

**Example: CONNECT_ACK (16 bytes total)**
```cpp
struct ConnectAckPacket {
    static constexpr PacketType type = PacketType::ConnectAck;
    static constexpr size_t PAYLOAD_SIZE = 4;
    
    PlayerId player_id;              // 1 byte
    uint8_t status;                   // 1 byte (0=OK, 1=Full, 2=BadName, 3=InGame)
    std::array<uint8_t, 2> reserved; // 2 bytes padding
    
    void serialize(PacketBuffer& buffer) const {
        buffer.write_header(make_header());  // 12 bytes
        buffer.write_uint8(player_id.value); // 1 byte
        buffer.write_uint8(status);          // 1 byte
        buffer.write_uint8(reserved[0]);     // 1 byte
        buffer.write_uint8(reserved[1]);     // 1 byte
    }
};
```

#### UDP Packets (Refactored)

UDP packets include `tick_id` in header for synchronization:

**PLAYER_INPUT (16 bytes total)**
```cpp
struct PlayerInputPacket {
    InputFlags inputs;                    // 1 byte bitmask
    std::array<uint8_t, 3> reserved;     // 3 bytes padding
    
    void serialize(PacketBuffer& buffer, uint32_t tick_id) const {
        buffer.write_header(make_header(tick_id));  // tick_id in header
        buffer.write_uint8(inputs.value);
        // ... reserved bytes
    }
};
```

**WORLD_SNAPSHOT (variable size)**
- Header: 12 bytes (with tick_id, packet_index, packet_count for fragmentation)
- Payload header: 4 bytes (entity_count u16 + reserved u16)
- Entity data: 12 bytes per entity (aligned)

```cpp
struct EntityState {
    EntityId entity_id;    // 4 bytes
    uint8_t entity_type;   // 1 byte
    uint8_t reserved;      // 1 byte padding
    uint16_t pos_x;        // 2 bytes (normalized 0..65535)
    uint16_t pos_y;        // 2 bytes (normalized 0..38864)
    uint16_t angle;        // 2 bytes (degrees 0..360)
};
// Total per entity: 12 bytes
```

### 5. PacketFactory Updates

**Before:** Read uint16 type first, then deserialize payload

**After:** 
1. Read 12-byte header first
2. Validate payload size matches buffer
3. Dispatch based on OpCode (uint8)
4. Return `PacketParseResult` including parsed header for tick/fragmentation info

```cpp
struct PacketParseResult {
    bool success;
    PacketVariant packet;
    CommonHeader header;  // NEW: Includes tick_id, packet_index, etc.
    std::string error;
};
```

### 6. Serialization Helpers

Added to `PacketBuffer`:
```cpp
void write_header(const CommonHeader& header);
CommonHeader read_header();
```

These methods handle the 12-byte header serialization/deserialization with proper little-endian encoding.

## Files Modified

1. **server/include/server/PacketBuffer.hpp**
   - Added `CommonHeader` struct (12 bytes, packed)
   - Added `write_header()` and `read_header()` methods

2. **server/include/server/PacketTypes.hpp**
   - Changed `PacketType` enum from `uint16_t` to `uint8_t`
   - Added all TCP opcodes (0x01-0x07)
   - Updated UDP opcodes to new values (0x10, 0x20, 0x21)
   - Changed `PlayerId` from `uint32_t` to `uint8_t`
   - Updated `InputFlags` constant names to match RFC (UP, DOWN, LEFT, RIGHT, SHOOT)

3. **server/include/server/Packets.hpp**
   - **Completely rewritten** with RFC-compliant structures
   - Added all 7 TCP packet types (ConnectReq, ConnectAck, DisconnectReq, NotifyDisconnect, GameStart, GameEnd, ReadyStatus)
   - Refactored UDP packets (PlayerInput, WorldSnapshot, PlayerStats)
   - Removed old packets (PlayerShoot, SpawnEntity, DestroyEntity, Heartbeat)
   - All packets now use `make_header()` to generate CommonHeader
   - Serialize methods write header first, then payload

4. **server/include/server/PacketFactory.hpp**
   - Updated `PacketVariant` to include new TCP packets
   - `deserialize_packet()` now reads header first, validates payload size
   - Returns `CommonHeader` in result for tick/fragmentation handling
   - `serialize_packet()` accepts tick parameters for UDP packets

## Backward Compatibility

⚠️ **BREAKING CHANGES:** This refactor is NOT backward compatible with old packet format.

- Old clients/servers using uint16 opcodes will fail to parse new packets
- Network protocol version should be incremented
- Consider implementing version handshake in CONNECT_REQ/ACK if needed

## Testing

### Compilation Test Results

```
CONNECT_REQ serialized: 44 bytes (expected 44 = 12 header + 32 payload) ✓
CONNECT_ACK serialized: 16 bytes (expected 16 = 12 header + 4 payload) ✓
PLAYER_INPUT serialized: 16 bytes (expected 16 = 12 header + 4 payload) ✓
WORLD_SNAPSHOT serialized: 40 bytes (expected 40 = 12 header + 4 + 24) ✓
Deserialization SUCCESS ✓
  OpCode: 0x1 ✓
  Payload size: 32 ✓
  Username: Alice ✓

OpCode sizes:
  PacketType enum: 1 byte (expected 1) ✓
  PlayerId: 1 byte (expected 1) ✓
  CommonHeader: 12 bytes (expected 12) ✓
```

### Required Unit Tests (TODO)

1. **Header Tests**
   - Serialize/deserialize header
   - Verify packed size is exactly 12 bytes
   - Test all field values (opcode, payload_size, tick_id, etc.)

2. **TCP Packet Tests**
   - All 7 TCP packet types
   - Username handling (null termination, length limits)
   - Status codes
   - Round-trip serialization

3. **UDP Packet Tests**
   - Input bitmask combinations
   - Entity state encoding/decoding
   - Coordinate normalization
   - Large snapshots with multiple entities

4. **Fragmentation Tests**
   - Multiple packet_index values
   - Reassembly logic
   - Out-of-order delivery

5. **Error Handling Tests**
   - Buffer too small
   - Invalid opcode
   - Payload size mismatch
   - Malformed data

6. **PacketFactory Tests**
   - Deserialize all packet types
   - Error messages for invalid packets
   - Variant type switching

## Coordinate Encoding (RFC Section 6.2)

World coordinates are normalized:
- **X axis:** 0..65535 (u16)
- **Y axis:** 0..38864 (u16, maintains 16:9 aspect ratio)

Client conversion formula:
```cpp
screen_x = (network_x * screen_width) / 65535;
screen_y = (network_y * screen_height) / 38864;
```

Example for 1920x1080:
```cpp
screen_x = (pos_x * 1920) / 65535;
screen_y = (pos_y * 1080) / 38864;
```

## Fragmentation Support (RFC Section 4.1)

For large snapshots that exceed MTU (~1472 bytes UDP):

1. Server splits entities across multiple packets
2. Each packet has same `tick_id` but different `packet_index` (0..N-1)
3. All packets include `packet_count = N`
4. Client reassembles when all N packets received for that tick_id

**Example:** 200 entities × 12 bytes = 2400 bytes payload
- Packet 0: Header (tick=100, index=0, count=2) + entities 0-100
- Packet 1: Header (tick=100, index=1, count=2) + entities 101-199

## Next Steps

1. ✅ Implement TCP packet structures
2. ✅ Refactor UDP packets for RFC compliance
3. ✅ Update PacketFactory deserialization
4. ✅ Add header serialization helpers
5. ✅ Verify compilation and basic serialization
6. ⏳ Update Network.hpp to use new packet API
7. ⏳ Write comprehensive unit tests
8. ⏳ Implement server-side TCP handler
9. ⏳ Implement client-side ASIO network layer
10. ⏳ Test full client-server handshake

## Questions for Future Development

1. **Version Handshake:** Should we add a protocol version field in CONNECT_REQ?
2. **Compression:** Large snapshots might benefit from compression (zlib/lz4)?
3. **Encryption:** RFC mentions plaintext vulnerability - add TLS/DTLS layer?
4. **Sequence Numbers:** Should we add sequence numbers to TCP packets for ordering?
5. **ACKs:** Do we need explicit ACK packets for reliable delivery confirmation?

## References

- **RFC Document:** `docs/docs/network-rfc.txt` (v3.2.0)
- **Protocol Spec:** `docs/docs/protocol.md`
- **Issue Template:** `.github/ISSUE_TEMPLATE/phase0-issue3-tcp-packets.md`
