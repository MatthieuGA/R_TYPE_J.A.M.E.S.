# R-Type Network Protocol Specification

> **Version:** 3.3.0
> **Last Updated:** 15th December 2025

## Table of Contents

1. [Overview](#1-overview)
   1. [Strategies](#11-strategies)
   2. [Data Types](#12-data-types)
   3. [Data Definitions](#13-data-definitions)
   4. [OpCode Categorization](#14-opcode-categorization)
2. [Error Handling Strategy](#2-error-handling-strategy)
   1. [Malformed Packets (UDP & TCP)](#21-malformed-packets-udp--tcp)
   2. [Logical Errors (TCP)](#22-logical-errors-tcp)
   3. [Desynchronization (UDP)](#23-desynchronization-udp)
3. [Packet Structure](#3-packet-structure)
   1. [Common Header](#31-common-header)
4. [TCP Commands (Session Management)](#4-tcp-commands-session-management)
   - [`0x01` CONNECT_REQ](#0x01---connect_req)
   - [`0x02` CONNECT_ACK](#0x02---connect_ack)
   - [`0x03` DISCONNECT_REQ](#0x03---disconnect_req)
   - [`0x04` NOTIFY_DISCONNECT](#0x04---notify_disconnect)
   - [`0x05` GAME_START](#0x05---game_start)
   - [`0x06` GAME_END](#0x06---game_end)
   - [`0x07` READY_STATUS](#0x07---ready_status)
   - [`0x08` NOTIFY_CONNECT](#0x08---notify_connect)
5. [UDP Commands (Real-time Gameplay)](#5-udp-commands-real-time-gameplay)
   - [`0x10` PLAYER_INPUT](#0x10---player_input)
   - [`0x20` WORLD_SNAPSHOT](#0x20---world_snapshot)
   - [`0x21` PLAYER_STATS](#0x21---player_stats)
6. [Security Considerations](#6-security-considerations)

## 1. Overview

This document defines the binary communication protocol used between the R-Type Client and Server.
The architecture follows a **Server Authoritative** model with **Snapshot Interpolation**.

### 1.1 Strategies

- **Snapshot State:** The server sends the full state of visible entities frequently. The client infers Spawn (new ID appears) and Death (ID disappears) from this list.
- **Client Prediction:** Visual effects (charging, movement initiation) start immediately on client input, but logic validation happens on server state reception.

### 1.2 Data Types

- `u8`, `u16`, `u32`: Unsigned integers (1, 2, 4 bytes).
- `f32`: 32-bit Float (IEEE 754).
- **Endianness:** Little Endian.

### 1.3 Data Definitions

To ensure strict binary compatibility, the following field types and sizes are defined for the entire protocol.

| Field Name | Type | Size (Bytes) | Range | Description |
| :--- | :--- | :--- | :--- | :--- |
| `OpCode` | `u8` | 1 | 0..255 | Command ID |
| `PayloadSize` | `u16` | 2 | 0..65535 | Size of following data |
| `TickId` | `u32` | 4 | 0..4294967295 | Frame counter |
| `PacketIndex` | `u8` | 1 | 0..255 | Fragment index |
| `PacketCount` | `u8` | 1 | 0..255 | Total fragments for Tick |
| `Reserved` | - | Varies | - | Padding bytes, MUST be set to 0 |
| `PlayerId` | `u8` | 1 | 0..255 | Session ID |
| `EntityId` | `u32` | 4 | 0..4294967295 | Unique Object ID (ECS) |
| `Status` | `u8` | 1 | 0..255 | Error/Success Codes (0=OK) |
| `Inputs` | `u8` | 1 | 0..255 | Bitmask (Keys held down) |
| `Score` | `u32` | 4 | 0..4294967295 | Game Score |
| `Lives` | `u8` | 1 | 0..255 | Player Life Count |
| `Angle` | `u16` | 2 | 0..360 | Degrees |
| `Position (X/Y)` | `u16` | 2 | 0..65535 | Normalized Coordinate |
| `Type` | `u8` | 1 | 0..255 | Sprite/Prefab ID |
| `Username` | `char` | 32 | 0..255 | Fixed-size String (Null-term) |

**Alignment Note:** Structures and Packets are padded to align on 4-byte boundaries where possible to prevent compiler-specific packing issues.

### 1.4 OpCode Categorization

Command IDs (OpCodes) are grouped by protocol and direction to facilitate debugging and parsing.

| Range | Protocol | Direction | Category |
| :--- | :--- | :--- | :--- |
| `0x00` - `0x3F` | **TCP** | Bi-directional | **Session Management** (Login, Lobby, Game State). |
| `0x40` - `0x7F` | **UDP** | Client -> Server | **Client Inputs** (Inputs, Actions). |
| `0x80` - `0xBF` | **UDP** | Server -> Client | **Server Snapshots** (World State, Stats, Events). |

---

## 2. Error Handling Strategy

### 2.1 Malformed Packets (UDP & TCP)

If a packet has an invalid Header, a `PayloadSize` mismatch, or an unknown `OpCode`:

- **Server Behavior:** **Silent Discard**. The server MUST NOT reply to avoid bandwidth flooding or amplification attacks.
- **Client Behavior:** Ignored.

### 2.2 Logical Errors (TCP)

Connection logic errors are handled explicitly in the `CONNECT_ACK` packet via the `Status` field.

### 2.3 Desynchronization (UDP)

If the Client and Server disagree on a position (e.g., due to lag or cheating), the Server's `WORLD_SNAPSHOT` is the absolute authority. The Client must override its local state with the received snapshot data.

---

## 3. Packet Structure

### 3.1 Common Header

**Crucial:** This header is prepended to **EVERY** packet sent over the network (TCP and UDP).

**Total Header Size:** 12 bytes.

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `OpCode` | `u8` | 1 | Unique command identifier. |
| `PayloadSize` | `u16` | 2 | Size of the data following this header. |
| `PacketIndex`| `u8` | 1 | Index of this packet within the Tick (0, 1, 2...). |
| `TickId` | `u32` | 4 | The game frame number. Used for UDP reassembly. Set to 0 for TCP. |
| `PacketCount`| `u8` | 1 | Total number of packets expected for this Tick. |
| `Reserved` | `u8[3]` | 3 | **Padding to align Header to 12 bytes.** |

#### TickId Usage

- The **TickId** is primarily used for UDP packets to manage **out-of-order delivery** and reassembly.
- TickId of the client and server are **not made to be synchronized**, they are only **identifiers** for packet grouping.
- It is used for **polish movements** for the client, and used as a **SequenceId** for the server.

#### Fragmentation Logic

- The server may send **large snapshots split** across multiple UDP packets.
- Clients must **reassemble** them based on `TickId`, `PacketIndex`, and `PacketCount`.
- A "Game Frame" is considered complete only when the client has received `PacketCount` unique packets for the same `TickId`.
- If a newer `TickId` is received fully, older partial Ticks are discarded.
- If the packet is sent into only one piece, `PacketIndex` and `PacketCount` are always `0` and `1` respectively.

---

## 4. TCP Commands (Session Management)

**Protocol:** TCP (Reliable, Ordered).
**Usage:** Lobby, Connection, Game State transitions.

### `0x01` - CONNECT_REQ

**Direction:** Client -> Server
**Description:** First packet sent by a client to join the server.

**Payload:** _32 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `Username` | `char[32]` | 32 | The desired username of the player (ASCII, null-terminated). |

- **When to send:** When the user clicks "Connect" on the main menu.
- **Server Behavior:** Checks if lobby is full. If not, assigns a `PlayerId`.

### `0x02` - CONNECT_ACK

**Direction:** Server -> Client
**Description:** Response to the connection request.

**Payload:** _4 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `PlayerId` | `u8` | 1 | The unique ID assigned to this client (e.g., 1, 2, 3, 4). |
| `Status` | `u8` | 1 | **Status Codes:**<br />`0` = OK (Success)<br />`1` = Server Full<br />`2` = Bad Username<br />`3` = Game in Progress |
| `Reserved` | `u8[2]` | 2 | Padding to align with 4 bytes. |

- **Client Behavior:** If Status is `0`, proceed to Lobby. Otherwise, show error message.

### `0x03` - DISCONNECT_REQ

**Direction:** Client -> Server
**Description:** Notifies the server that the client is closing the game gracefully.

**Payload:** _0 bytes_

- **When to send:** When user clicks "Quit" or closes the window.

### `0x04` - NOTIFY_DISCONNECT

**Direction:** Server -> Client
**Description:** Broadcasted to all connected clients when someone leaves.

**Payload:** _4 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `PlayerId` | `u8` | 1 | The ID of the player who disconnected. |
| `Reserved` | `u8[3]` | 3 | Padding to align with 4 bytes. |

- **Client Behavior:** Remove the corresponding player sprite/UI element from the lobby or game.

### `0x05` - GAME_START

**Direction:** Server -> Client
**Description:** Sent when the game is starting. Upon sending/receiving this command, the `TickId` counter **MUST** be reset to 0 for the new match.

**Payload:** _4 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `ControlledEntityId` | `u32` | 4 | The EntityID of the spaceship assigned to this player. The client should track this ID for camera/HUD. |

### `0x06` - GAME_END

**Direction:** Server -> Client
**Description:** Sent when the game has ended.

**Payload:** _4 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `WinningPlayerId` | `u8` | 1 | ID of the winning player (or 0 for draw). |
| `Reserved` | `u8[3]` | 3 | Padding to align with 4 bytes. |

### `0x07` - READY_STATUS

**Direction:** Client -> Server
**Description:** Sent by the client to indicate readiness in the lobby.

**Payload:** _1 byte_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `IsReady` | `u8` | 1 | `0` = Not Ready, `1` = Ready. |
| `Reserved` | `u8[3]` | 3 | Padding to align with 4 bytes. |

- **When to send:** When the user toggles the "Ready" button in the lobby.

When all the players in the lobby have sent `IsReady = 1`, the server automatically starts the game by sending GAME_START to all clients.

### `0x08` - NOTIFY_CONNECT

**Direction:** Server -> Client
**Description:** Notify clients of a new player's connection.
**Payload:** _36 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `PlayerId` | `u8` | 1 | The unique ID assigned to the new player. |
| `Reserved` | `u8[3]` | 3 | Padding to align with 4 bytes. |
| `Username` | `char[32]` | 32 | The username of the newly connected player (ASCII, null-terminated). |

- **Client Behavior:** Add the new player to the lobby UI.
- **When to send:** Immediately after processing a successful CONNECT_REQ from a new client.

---

## 5. UDP Commands (Real-time Gameplay)

**Protocol:** UDP (Fast, Unreliable).
**Usage:** Inputs, Positions, Physics.

### `0x10` - PLAYER_INPUT

**Direction:** Client -> Server
**Freq:** Every client tick (e.g., 60Hz).
**Description:** Contains the current state of the controller/keyboard.

**Payload:** _4 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `Inputs` | `u8` | 1 | Bitmask of HELD keys. |
| `Reserved` | `u8[3]` | 3 | Padding to align with 4 bytes. |

**Bitmask Values:**

Combine these values using bitwise OR (`|`) to set multiple keys (e.g., UP + SHOOT).

| Hex | Decimal | Key |
| :--- | :--- | :--- |
| `0x01` | 1 | UP |
| `0x02` | 2 | DOWN |
| `0x04` | 4 | LEFT |
| `0x08` | 8 | RIGHT |
| `0x10` | 16 | SHOOT |

**Charge Shot Logic (State vs Event):**
The protocol does not send "Press" or "Release" events. It sends the "Hold" state. The server detects the shot logic by comparing the current tick's state with the previous one.

**Example Timeline (Processing Logic):**

| Tick | Client Action | Bitmask sent | Packet Status | Server Logic & Action |
| :--- | :--- | :--- | :--- | :--- |
| 100 | Nothing | `0` (`00000`) | Received | `0` -> `0`. No change. |
| 101 | **Press** Shoot | `16` (`10000`) | Received | `0` -> `16`. **Start Charge Timer.** |
| 102 | Hold Shoot | `16` (`10000`) | **LOST** ❌ | (Server assumes state persists or waits). |
| 103 | Hold Shoot | `16` (`10000`) | Received | `16` -> `16`. Continue Charge. |
| ... | ... | ... | ... | ... |
| 120 | **Release** Shoot | `0` (`00000`) | Received | `16` -> `0`. **FIRE TRIGGERED!** |

_Note: Even if packet 102 was lost, the server recovers the state at tick 103. The shot is triggered only when the bit returns to 0._

### `0x20` - WORLD_SNAPSHOT

**Direction:** Server -> Client
**Freq:** 60Hz.
**Description:** The "Source of Truth". Contains the state of every active entity.

**Payload:** _4 bytes (Header) + N * 12 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `EntityCount` | `u16` | 2 | Number of entities **in this specific packet**. |
| `Reserved` | `u8[2]` | 2 | Padding to align with 4 bytes. |
| `Entities` | `Array` | Var | List of `EntityState` structures. |

**Client Logic:**

1. **Reassemble:** Collect all parts for `TickId`.
   - **If complete:** Process Snapshot. If any older incomplete ticks are pending, delete them.
   - **If incomplete:** Wait for missing parts (or discard if too old).
2. **Compare:** Compare received Entity IDs with local Entity IDs.
   - **New ID:** Spawn Entity.
   - **Missing ID:** Destroy Entity.
   - **Existing ID:** Interpolate position.

#### Structure: `EntityState` (Repeating block)

**Size:** 12 bytes per entity.

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `EntityId` | `u32` | 4 | Unique ID. |
| `Type` | `u8` | 1 | Sprite Type (1=P1, 2=Enemy...). |
| `Reserved` | `u8` | 1 | Padding to align with 4 bytes. |
| `PosX` | `u16` | 2 | **X Position (Normalized).** 0 = Left, 65535 = Right. |
| `PosY` | `u16` | 2 | **Y Position (Normalized).** 0 = Top, 65535 = Bottom. |
| `Angle` | `u16` | 2 | Rotation in degrees (0-360). |

**Note on Coordinates:**
The game world is defined on a fixed coordinate system of **65535 x 38864** units (approximately 16:9 aspect ratio).
The client is responsible for mapping these coordinates to its local screen resolution using the rule of three.

Example for a client with a 1920x1080 screen:

- `ScreenX = (NetworkX * 1920) / 65535`
- `ScreenY = (NetworkY * 1080) / 38864`

### `0x21` - PLAYER_STATS

**Direction:** Server -> Client
**Freq:** On change.
**Description:** Updates HUD information.

**Payload:** _8 bytes_

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `PlayerId` | `u8` | 1 | ID of the player to update. |
| `Lives` | `u8` | 1 | Remaining lives. |
| `Reserved` | `u8[2]` | 2 | Padding to align with 4 bytes. |
| `Score` | `u32` | 4 | Current score. |

## 6. Security Considerations

### 6.1 Plaintext Communication

This protocol does not implement encryption (TLS/SSL). All data, including usernames and game states, is transmitted in **cleartext**.

- **Risk:** The protocol is susceptible to Packet Sniffing and Man-in-the-Middle (MITM) attacks.
- **Requirement:** Do not send sensitive data (real passwords, personal info) over this protocol.

### 6.2 Input Validation (Anti-Cheat)

The Server is the authoritative source of truth and **MUST** validate all incoming packets.

- **Trust No One:** The server must not blindly accept game logic from the client (e.g., fire rate, movement speed, cooldowns). The server simulates the input and validates the result.
- **Sanitization:** The `Username` field in `CONNECT_REQ` must be sanitized to prevent buffer overflows, display exploits, or injection attacks in the server logs/UI.

### 6.3 Denial of Service (DoS) Mitigation

The **Silent Discard** policy defined in the Error Handling section is a security measure.

- By not replying to malformed packets, the server prevents **Amplification Attacks**.
- It reduces the CPU load when under a flooding attack, as the server stops processing the packet immediately after the header check fails.
