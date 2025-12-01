# AGENTS.md

## Project Overview

**R-Type J.A.M.E.S.** is a modern C++23 recreation of the classic R-Type arcade game. It features a custom Entity-Component-System (ECS) engine, a multithreaded authoritative server, and an SFML-based client.

**Project Phases:**

- **Part 1 (Prototype):** Core engine, authoritative server, UDP networking, 4-player gameplay.
- **Part 2 (Advanced):** Standalone engine, advanced networking (lobbies), and advanced gameplay.

**Key Technologies:**

- **Language:** C++23
- **Build System:** CMake (3.23+)
- **Dependency Manager:** vcpkg
- **Graphics:** SFML (2.6.1+)
- **Networking:** Asio (via vcpkg)
- **Testing:** GoogleTest
- **Documentation:** Docusaurus

**Architecture:**

- `engine/`: Core ECS library (header-only/static lib).
- `client/`: Game client (SFML, Input, Audio).
- `server/`: Game server (Authoritative logic, UDP networking).
- `tests/`: Unit and functional tests.

## Technical Constraints

- **Protocol:** MUST be Binary. UDP for gameplay (entities, movements, events). TCP allowed only with justification.
- **Game Loop:** MUST use timers/deltas. NOT tied to CPU speed.
- **Engine:** NO existing game engines (Unity, Unreal, Godot, etc.).
- **Server:** MUST be authoritative and multithreaded. MUST handle client crashes gracefully.
- **Client:** MUST be graphical (SFML/SDL/Raylib).

## Setup Commands

- **Prerequisites:**
  - C++ Compiler (GCC 7+, MSVC 2019+, or Clang 11+)
  - CMake 3.23+
  - vcpkg (Set `VCPKG_ROOT` env var or use the submodule)
  - Node.js & npm (for hooks and docs)

- **Install Dependencies (Hooks & Docs):**

  ```bash
  npm install
  ```

- **Bootstrap vcpkg (if not installed):**

  ```bash
  ./vcpkg/bootstrap-vcpkg.sh
  ```

## Development Workflow

### Build

**Using Scripts (Recommended):**

- **Linux:** `./build.sh`
- **Windows:** `build.bat`

**Manual CMake Build:**

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Run

- **Client:**

  ```bash
  ./build/client/r-type_client [server-ip] [port]
  ```

- **Server:**

  ```bash
  ./build/server/r-type_server [port]
  ```

## Testing Instructions

- **Run All Tests:**

  ```bash
  cd build
  ctest --output-on-failure
  ```

  Or use the script: `./run_tests.bat` (Windows)

- **Run Coverage (Linux/GCC/Clang):**
  Requires `gcovr`.

  ```bash
  cmake -S . -B build -DENABLE_COVERAGE=ON ...
  cmake --build build
  cd build
  make run-coverage
  ```

  Report generated at `build/coverage.html`.

### Network Simulation Testing

- **Linux:** Use `netem` to simulate lag/loss.
- **Windows:** Use `clumsy`.
- **Scenarios to Test:**
  - 2% packet drop/duplication.
  - Latency > 150ms.
  - Low bandwidth (5KB/sec).

## Code Style

- **Standard:** C++23
- **Formatter:** `clang-format` (Google Style)
- **Linting:** `clang-tidy`, `cpplint`
- **Commit Messages:** Gitmoji + English (e.g., `✨ [Engine] Add component`). Enforced by `commitlint`.

**Pre-commit Hooks:**
Husky and lint-staged automatically format code and check commit messages.

- Run hooks manually: `npm run prepare`

## Build and Deployment

- **Output Directories:**
  - Client Binary: `build/client/r-type_client` (Linux) / `r-type_client.exe` (Windows)
  - Server Binary: `build/server/r-type_server` (Linux) / `r-type_server.exe` (Windows)
  - Assets: Copied to `Assets/` next to the binary.

- **Documentation Build:**

  ```bash
  cd docs
  npm install
  npm run build
  ```

## Documentation Requirements

- **Language:** English.
- **Required Documents:**
  - **Developer Docs:** Architecture, Systems, Tutorials.
  - **Technical & Comparative Study:** Justification of choices (Algo, Storage, Security).
  - **Protocol RFC:** Binary protocol specification.
  - **Accessibility:** Solutions for Physical, Visual/Audio, and Cognitive disabilities.

## Pull Request Guidelines

- **Branch Naming:**
  - `feature/<issue-id>-<description>`
  - `bugfix/<issue-id>-<description>`
  - `refactor/<issue-id>-<description>`
  - `docs/<issue-id>-<description>`

- **Process:**
  1. Create an Issue.
  2. Create a Branch from `main` (or `Epic-...` branch).
- **Assets:** Place assets in `assets/` folder. They are copied to the build directory.

## Advanced Goals (Part 2)

- **Standalone Engine:** The engine should be reusable for a second game (e.g., Pong, Mario).
- **Advanced Networking:** Multi-instance server, Lobbies, Chat, User accounts.
- **Engine Features:** Scripting (Lua/Python), Physics, Resource Management, Scene Editor.
  4. Open PR with description and link to issue (`Closes #ID`).
  5. Ensure CI passes (Build, Tests, Format).
  6. Squash & Merge.

## Additional Notes

- **ECS Architecture:** Components must be Plain Old Data (POD). Systems contain logic.
- **Networking:** UDP for gameplay, TCP (optional/planned) for reliable control.
- **Assets:** Place assets in `assets/` folder. They are copied to the build directory.
