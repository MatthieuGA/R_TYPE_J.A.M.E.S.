---
sidebar_position: 3
---

# Network Protocol

## Overview

R-TYPE J.A.M.E.S. uses **UDP (User Datagram Protocol)** for network communication, optimized for low-latency, real-time multiplayer gaming.

## Why UDP?

- **Low Latency**: No connection overhead or acknowledgment delays
- **Real-time**: Perfect for fast-paced action games
- **Custom Reliability**: We implement application-level reliability where needed

## Packet Structure

### General Packet Format

All packets follow this basic structure:

```
┌─────────────────────────────────────────────────┐
│  Header (8 bytes)                               │
├─────────────────────────────────────────────────┤
│  Payload (variable size)                        │
└─────────────────────────────────────────────────┘
```

### Header Format

```cpp
struct PacketHeader {
    uint16_t packet_type;    // Type of packet
    uint16_t sequence_num;   // For ordering and reliability
    uint32_t timestamp;      // Client/server timestamp
};
```

## Packet Types

### Client to Server

#### 1. Input Packet (0x0001)
Sent when the player provides input.

```cpp
struct InputPacket {
    PacketHeader header;
    uint8_t input_flags;     // Bit flags for keys pressed
    float mouse_x;           // Mouse X position
    float mouse_y;           // Mouse Y position
};
```

**Input Flags:**
- Bit 0: Move Up
- Bit 1: Move Down
- Bit 2: Move Left
- Bit 3: Move Right
- Bit 4: Shoot
- Bit 5: Special Ability

#### 2. Connect Request (0x0002)
Initial connection request from client.

```cpp
struct ConnectRequest {
    PacketHeader header;
    uint32_t protocol_version;
    char player_name[32];
};
```

#### 3. Disconnect (0x0003)
Client gracefully disconnecting.

```cpp
struct DisconnectPacket {
    PacketHeader header;
    uint32_t player_id;
};
```

### Server to Client

#### 1. Game State Update (0x1001)
Regular game state snapshots.

```cpp
struct GameStatePacket {
    PacketHeader header;
    uint32_t num_entities;
    EntitySnapshot entities[];
};

struct EntitySnapshot {
    uint32_t entity_id;
    float pos_x, pos_y;
    float vel_x, vel_y;
    uint16_t sprite_id;
    uint8_t health;
};
```

#### 2. Connect Response (0x1002)
Server's response to connection request.

```cpp
struct ConnectResponse {
    PacketHeader header;
    uint32_t player_id;
    bool accepted;
    char reason[64];        // If rejected, reason why
};
```

#### 3. Entity Spawned (0x1003)
Notify clients of new entities.

```cpp
struct EntitySpawnedPacket {
    PacketHeader header;
    uint32_t entity_id;
    uint16_t entity_type;
    float pos_x, pos_y;
};
```

#### 4. Entity Destroyed (0x1004)
Notify clients of destroyed entities.

```cpp
struct EntityDestroyedPacket {
    PacketHeader header;
    uint32_t entity_id;
};
```

## Reliability Mechanisms

### Acknowledgments

Critical packets (connect, disconnect, spawn) require acknowledgment:

```cpp
struct AckPacket {
    PacketHeader header;
    uint16_t acked_sequence_num;
};
```

### Retransmission

- Reliable packets are resent if not acknowledged within 100ms
- Maximum 3 retransmission attempts
- After 3 failures, connection is considered lost

### Sequence Numbers

- Each packet has a sequence number
- Used to detect packet loss and reordering
- Clients/server track last received sequence number

## Client-Side Prediction

To minimize perceived latency:

1. **Predict Movement**: Client simulates player movement locally
2. **Server Reconciliation**: When server state arrives, reconcile with prediction
3. **Entity Interpolation**: Smooth movement of other entities

### Prediction Algorithm

```cpp
// Client predicts next state
predicted_state = current_state + input * delta_time;

// Server sends authoritative state
authoritative_state = server_snapshot;

// Reconcile
if (predicted_state != authoritative_state) {
    // Smooth interpolation to correct state
    current_state = lerp(predicted_state, authoritative_state, 0.1);
}
```

## Update Frequency

- **Server tick rate**: 60 Hz (every ~16.67ms)
- **Client input rate**: As fast as possible (typically 60+ Hz)
- **State broadcast rate**: 20 Hz (every 50ms) with delta compression

## Bandwidth Optimization

### Delta Compression

Only send changed data:
- Track previous state
- Send only differences
- Reduces bandwidth by ~70%

### Interest Management

Server only sends entities that are:
- Within player's viewport
- Relevant to gameplay (e.g., nearby enemies)

## Error Handling

### Packet Loss
- Non-critical packets: Ignore loss (next update will overwrite)
- Critical packets: Retransmit with timeout

### Out-of-Order Packets
- Use sequence numbers to detect
- Buffer and reorder if necessary
- Discard very old packets

### Connection Timeout
- Client timeout: 5 seconds without server response
- Server timeout: 10 seconds without client input

## Security Considerations

- **Input Validation**: Server validates all client inputs
- **Rate Limiting**: Prevent packet flooding
- **Authoritative Server**: Server is always the source of truth
- **Anti-Cheat**: Server-side validation prevents speed hacks and teleportation

## Example Communication Flow

```
Client                           Server
  │                                │
  ├──── Connect Request ──────────►│
  │                                │
  │◄──── Connect Response ─────────┤
  │                                │
  ├──── Input (Move Right) ───────►│
  │                                │
  │◄──── Game State Update ────────┤
  │      (Player position updated) │
  │                                │
  ├──── Input (Shoot) ────────────►│
  │                                │
  │◄──── Entity Spawned ───────────┤
  │      (Bullet created)          │
  │                                │
  │◄──── Game State Update ────────┤
  │      (Bullet position)         │
  │                                │
  ├──── Disconnect ───────────────►│
  │                                │
```

## Implementation Notes

- Use **ASIO** or **Boost.Asio** for asynchronous networking
- Serialize packets with **Protocol Buffers** or custom binary serialization
- Consider **ENet** library for high-level UDP abstraction

## Next Steps

- Explore the [Architecture](./architecture.md) overview
- Learn about entity synchronization strategies
- Read about lag compensation techniques
