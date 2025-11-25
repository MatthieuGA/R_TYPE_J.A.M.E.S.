---
sidebar_position: 2
---

# Architecture

## Overview

R-TYPE J.A.M.E.S. follows a **client-server architecture** with a custom game engine built on the **Entity Component System (ECS)** pattern.

## System Architecture

```
┌─────────────────┐                  ┌─────────────────┐
│                 │                  │                 │
│     Client      │ ◄─────────────► │     Server      │
│                 │   UDP Protocol   │                 │
└─────────────────┘                  └─────────────────┘
        │                                     │
        │                                     │
        ▼                                     ▼
┌─────────────────┐                  ┌─────────────────┐
│  Game Engine    │                  │  Game Engine    │
│   (ECS Core)    │                  │   (ECS Core)    │
└─────────────────┘                  └─────────────────┘
```

## Core Components

### 1. Game Engine

The game engine is the heart of R-TYPE J.A.M.E.S. It implements an Entity Component System pattern.

#### Entity Component System (ECS)

- **Entities**: Game objects represented by unique IDs
- **Components**: Data containers (Position, Velocity, Sprite, Health, etc.)
- **Systems**: Logic processors that operate on entities with specific components

**Key benefits:**
- High performance through cache-friendly data layout
- Flexible composition over inheritance
- Easy to add new features without modifying existing code

### 2. Client

The client is responsible for:
- Rendering graphics
- Handling user input
- Sending commands to the server
- Receiving and displaying game state updates
- Local prediction and interpolation

**Key modules:**
- **Renderer**: Graphics rendering using SFML/SDL
- **Input Manager**: Keyboard, mouse, and gamepad handling
- **Network Client**: Communication with the server

### 3. Server

The server is the authoritative source of truth for the game state:
- Processing client inputs
- Running game simulation
- Collision detection
- AI for enemies
- Broadcasting state updates to clients

**Key modules:**
- **Game Loop**: Fixed timestep simulation
- **Network Server**: UDP socket management
- **State Manager**: Game state synchronization
- **Physics Engine**: Collision and movement

## Directory Structure

```
R_TYPE_J.A.M.E.S/
├── engine/           # Core game engine (ECS)
│   ├── include/      # Engine headers
│   └── src/          # Engine implementation
├── client/           # Client application
│   └── main.cpp      # Client entry point
├── server/           # Server application
│   └── main.cpp      # Server entry point
├── tests/            # Unit and integration tests
├── docs/             # Documentation (Docusaurus)
└── CMakeLists.txt    # Build configuration
```

## Data Flow

### Client to Server
1. Player presses a key (e.g., move right)
2. Input is captured by Input Manager
3. Command packet is created
4. Packet is sent to server via UDP

### Server to Client
1. Server processes all client inputs
2. Game simulation updates (physics, AI, etc.)
3. New game state is computed
4. State snapshot is broadcast to all clients
5. Clients receive and render the new state

## Threading Model

- **Main Thread**: Game loop and rendering
- **Network Thread**: Asynchronous I/O for network communication
- **Asset Loading Thread**: Background loading of resources

## Performance Considerations

- Fixed timestep for deterministic simulation
- Delta time for smooth rendering
- Object pooling for frequent allocations
- Spatial partitioning for efficient collision detection

## Next Steps

- Learn about the [Network Protocol](./protocol.md)
- Explore the ECS implementation details
