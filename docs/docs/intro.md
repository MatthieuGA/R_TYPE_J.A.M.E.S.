---
sidebar_position: 1
---

# Getting Started

Welcome to the **R-TYPE J.A.M.E.S.** documentation!

R-TYPE J.A.M.E.S. is a modern C++ implementation of the classic R-TYPE game, featuring:

- **Client-Server Architecture**: Networked multiplayer gameplay
- **Custom Game Engine**: Built with modern C++ practices
- **ECS (Entity Component System)**: Efficient and scalable game architecture
- **Cross-platform**: Support for multiple platforms

## Prerequisites

Before you begin, ensure you have the following installed:

- **CMake** (version 3.20 or higher)
- **C++ Compiler** with C++17 support (GCC, Clang, or MSVC)
- **Git**

## Quick Start

### Building the Project

1. Clone the repository:
```bash
git clone https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S.git
cd R_TYPE_J.A.M.E.S.
```

2. Configure the project:
```bash
cmake -S . -B build
```

3. Build the project:
```bash
cmake --build build -j$(nproc)
```

### Running the Game

#### Start the Server
```bash
./build/server/r-type_server
```

#### Start the Client
```bash
./build/client/r-type_client
```

## Running Tests

Run the test suite:
```bash
cd build && ctest --output-on-failure
```

## Next Steps

- Learn about the [Architecture](./architecture.md) of the game
- Understand the [Network Protocol](./protocol.md)
- Explore the API documentation
