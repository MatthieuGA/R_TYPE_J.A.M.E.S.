# Technology Choices and Comparative Study

**Document Version:** 1.0.1
**Date:** December 9, 2025
**Team:** J.A.M.E.S. (Jocelyn, Arthuryan, Matthieu, Enoal, Samuel)

---

## Table of Contents

1. [Introduction](#introduction)
2. [Graphics Library: SFML](#graphics-library-sfml)
3. [Networking Library: Boost.Asio](#networking-library-boostasio)
4. [Dependency Manager: vcpkg](#dependency-manager-vcpkg)
5. [Conclusion](#conclusion)
6. [References](#references)

---

## Introduction

This document justifies the key technology choices for the R-Type project. Each selection is evaluated based on:

- **Project Requirements:** Binary UDP protocol, multithreaded server, cross-platform (Linux/Windows)
- **Performance:** Real-time 60 FPS rendering and low-latency networking
- **Team Expertise:** Leveraging existing knowledge to maximize productivity
- **Integration:** Seamless compatibility with our toolchain (CMake, C++20)

---

## Graphics Library: SFML

### Quick Comparison

| Library | Pros | Cons |
|---------|------|------|
| **SFML** ✅ | • Native C++ (RAII, OOP)<br />• **All 5 team members already proficient**<br />• Integrated (graphics + audio + input)<br />• Excellent 2D API<br />• vcpkg support | • Less control than raw OpenGL<br />• Larger binary than minimal libs |
| **SDL2** | • Industry standard<br />• Large ecosystem<br />• Better gamepad support | • C API (verbose in C++)<br />• Requires SDL_mixer for audio<br />• **3-5 days learning curve** |
| **Raylib** | • Very simple API<br />• Good for prototyping | • Less mature<br />• Limited for complex games<br />• **2-3 days learning curve** |
| **GLFW + OpenGL** | • Maximum performance<br />• Full GPU control | • Requires shader programming<br />• No 2D helpers<br />• **2 weeks learning curve** |

### Decision: SFML 2.6+

**Critical Factor:** All 5 team members have SFML experience from previous school projects.

**Impact:**

- **0 days** onboarding vs 2-5 days for alternatives
- Can focus on networking and ECS architecture immediately
- Familiar debugging and development workflow

**Why It Fits:**

- ✅ Cross-platform (Linux/Windows requirement)
- ✅ 60 FPS 2D rendering capability
- ✅ Modern C++20 compatible
- ✅ vcpkg integration

---

## Networking Library: Boost.Asio

### Quick Comparison

| Library | Pros | Cons |
|---------|------|------|
| **Boost.Asio** ✅ | • **Async I/O** (non-blocking)<br />• Cross-platform (Linux/Windows)<br />• Very mature/battle-tested<br />• Modern C++ (lambdas, RAII)<br />• Perfect for UDP binary protocol<br />• Part of Boost ecosystem<br />• Excellent documentation | • Requires Boost (~100MB+)<br />• Slower compilation<br />• Template-heavy |
| **Asio (standalone)** | • Same API as Boost.Asio<br />• Header-only option<br />• Smaller dependency | • Less mature than Boost version<br />• Moderate learning curve |
| **Raw Sockets** | • Maximum control<br />• No dependencies | • **Platform-specific** (#ifdef hell)<br />• Manual async I/O (epoll/IOCP)<br />• Complex cross-platform code |
| **SFML Network** | • Simple API<br />• Integrated with SFML | • **Blocking I/O only**<br />• Requires thread-per-client<br />• Poor for multithreaded server |
| **ZeroMQ** | • High-level patterns<br />• Message queuing | • **Higher latency**<br />• Overkill for direct client-server<br />• Not designed for real-time games |

### Decision: Boost.Asio

**Why Async I/O Matters:**

```cpp
// SFML (blocking): Thread waits idle for data
socket.receive(packet);  // BLOCKS ❌

// Boost.Asio (async): Thread handles other clients while waiting
socket.async_receive_from(buffer, endpoint,
    [](asio::error_code ec, size_t bytes) {
        // Called when data arrives ✅
    });
```

**Project Requirements:**

- ✅ Multithreaded server → Boost.Asio's `io_context` handles multiple clients per thread
- ✅ UDP networking → Native async UDP support
- ✅ Cross-platform → No `#ifdef` needed (abstracts epoll/IOCP/kqueue)
- ✅ Custom binary protocol → Low-level socket control

**Performance:**

- 1 thread handles 100+ clients (vs thread-per-client with blocking I/O)
- Non-blocking operations prevent slow clients from affecting others

---

## Dependency Manager: vcpkg

### Quick Comparison

| Tool | Pros | Cons |
|------|------|------|
| **vcpkg** ✅ | • **One-line CMake integration**<br />• Cross-platform (MSVC/GCC)<br />• Auto dependency resolution<br />• Binary caching<br />• Microsoft backing | • Slower first build<br />• Requires installation |
| **Conan** | • Mature versioning<br />• Pre-built binaries | • **Additional setup step**<br />• Python dependency<br />• Team unfamiliar |
| **FetchContent** | • Built into CMake<br />• No external tools | • **Rebuilds on clean**<br />• Manual transitive deps<br />• No caching<br />• Clutters CMakeLists.txt |
| **Git Submodules** | • Complete control<br />• Works offline | • **Manual config per lib**<br />• Large repo size<br />• Violates project spec* |

*Project spec: "Copying full dependencies source code into your repository is NOT proper dependency management"

### Decision: vcpkg (manifest mode)

**Manifest Example:**

```json
{
  "dependencies": ["sfml", "boost-asio"]
}
```

**CMake Integration:**

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
# That's it! No boilerplate in CMakeLists.txt
```

**Key Benefits:**

- ✅ Auto-resolves transitive deps (SFML → freetype → zlib → ...)
- ✅ Cross-platform builds (Windows MSVC + Linux GCC)
- ✅ Binary caching (CI/CD: 1 min vs 20 min)
- ✅ All needed packages available (SFML, Boost.Asio, GoogleTest)

---

## Conclusion

### Technology Stack Summary

| Layer | Technology | Key Reason |
|-------|-----------|------------|
| **Graphics** | SFML 2.6+ | **All 5 members proficient** → 0 days learning |
| **Networking** | Boost.Asio | Async I/O for multithreaded server |
| **Dependencies** | vcpkg | One-line CMake integration |

### Project Requirements Compliance

- ✅ **Binary UDP Protocol** → Boost.Asio provides low-level socket control
- ✅ **Multithreaded Server** → Boost.Asio io_context thread pool (non-blocking)
- ✅ **Cross-Platform (Linux/Windows)** → All technologies support both platforms seamlessly
- ✅ **Package Manager** → vcpkg with CMake integration
- ✅ **60 FPS Rendering** → SFML handles real-time 2D graphics

### Strategic Decisions

| Decision | Rationale | Impact |
|----------|-----------|--------|
| **SFML over alternatives** | Team already expert | Saves 3-14 days onboarding |
| **Boost.Asio over raw sockets** | Cross-platform async I/O | Eliminates platform-specific code |
| **vcpkg over Conan/FetchContent** | Simplest CMake integration | One-line configuration |

---

## References

### Official Documentation
 # Technology Choices and Comparative Study

**Document Version:** 1.0.1
**Date:** December 9, 2025
**Team:** J.A.M.E.S. (Jocelyn, Arthuryan, Matthieu, Enoal, Samuel)

---

## Table of Contents

1. [Introduction](#introduction)
2. [Graphics Library: SFML](#graphics-library-sfml)
3. [Networking Library: Boost.Asio](#networking-library-boostasio)
4. [Dependency Manager: vcpkg](#dependency-manager-vcpkg)
5. [Conclusion](#conclusion)
6. [References](#references)

---

## Introduction

This document justifies the key technology choices for the R-Type project. Each selection is evaluated based on:

- **Project Requirements:** Binary UDP protocol, multithreaded server, cross-platform (Linux/Windows)
- **Performance:** Real-time 60 FPS rendering and low-latency networking
- **Team Expertise:** Leveraging existing knowledge to maximize productivity
- **Integration:** Seamless compatibility with our toolchain (CMake, C++20)

---

## Graphics Library: SFML

### Quick Comparison

| Library | Pros | Cons |
|---------|------|------|
| **SFML** ✅ | • Native C++ (RAII, OOP)<br />• **All 5 team members already proficient**<br />• Integrated (graphics + audio + input)<br />• Excellent 2D API<br />• vcpkg support | • Less control than raw OpenGL<br />• Larger binary than minimal libs |
| **SDL2** | • Industry standard<br />• Large ecosystem<br />• Better gamepad support | • C API (verbose in C++)<br />• Requires SDL_mixer for audio<br />• **3-5 days learning curve** |
| **Raylib** | • Very simple API<br />• Good for prototyping | • Less mature<br />• Limited for complex games<br />• **2-3 days learning curve** |
| **GLFW + OpenGL** | • Maximum performance<br />• Full GPU control | • Requires shader programming<br />• No 2D helpers<br />• **2 weeks learning curve** |

### Decision: SFML 2.6+

**Critical Factor:** All 5 team members have SFML experience from previous school projects.

**Impact:**

- **0 days** onboarding vs 2-5 days for alternatives
- Can focus on networking and ECS architecture immediately
- Familiar debugging and development workflow

**Why It Fits:**

- ✅ Cross-platform (Linux/Windows requirement)
- ✅ 60 FPS 2D rendering capability
- ✅ Modern C++20 compatible
- ✅ vcpkg integration

---

## Networking Library: Boost.Asio

### Quick Comparison

| Library | Pros | Cons |
|---------|------|------|
| **Boost.Asio** ✅ | • **Async I/O** (non-blocking)<br />• Cross-platform (Linux/Windows)<br />• Very mature/battle-tested<br />• Modern C++ (lambdas, RAII)<br />• Perfect for UDP binary protocol<br />• Part of Boost ecosystem<br />• Excellent documentation | • Requires Boost (~100MB+)<br />• Slower compilation<br />• Template-heavy |
| **Asio (standalone)** | • Same API as Boost.Asio<br />• Header-only option<br />• Smaller dependency | • Less mature than Boost version<br />• Moderate learning curve |
| **Raw Sockets** | • Maximum control<br />• No dependencies | • **Platform-specific** (#ifdef hell)<br />• Manual async I/O (epoll/IOCP)<br />• Complex cross-platform code |
| **SFML Network** | • Simple API<br />• Integrated with SFML | • **Blocking I/O only**<br />• Requires thread-per-client<br />• Poor for multithreaded server |
| **ZeroMQ** | • High-level patterns<br />• Message queuing | • **Higher latency**<br />• Overkill for direct client-server<br />• Not designed for real-time games |

### Decision: Boost.Asio

**Why Async I/O Matters:**

Async I/O enables low-latency, non-blocking networking suitable for real-time games and server scalability.

### Dependency Management

| Tool | Pros | Cons |
|------|------|------|
| **vcpkg** ✅ | • **One-line CMake integration**<br />• Cross-platform (MSVC/GCC)<br />• Auto dependency resolution<br />• Binary caching<br />• Microsoft backing | • Slower first build<br />• Requires installation |
| **Conan** | • Mature versioning<br />• Pre-built binaries | • **Additional setup step**<br />• Python dependency<br />• Team unfamiliar |
| **FetchContent** | • Built into CMake<br />• No external tools | • **Rebuilds on clean**<br />• Manual transitive deps<br />• No caching<br />• Clutters CMakeLists.txt |
| **Git Submodules** | • Complete control<br />• Works offline | • **Manual config per lib**<br />• Large repo size<br />• Violates project spec* |

*Project spec: "Copying full dependencies source code into your repository is NOT proper dependency management"

### Decision: vcpkg (manifest mode)

**Manifest Example:**

```json
{
  "dependencies": ["sfml", "boost-asio"]
}
```

**CMake Integration:**

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
# That's it! No boilerplate in CMakeLists.txt
```

**Key Benefits:**

- ✅ Auto-resolves transitive deps (SFML → freetype → zlib → ...)
- ✅ Cross-platform builds (Windows MSVC + Linux GCC)
- ✅ Binary caching (CI/CD: 1 min vs 20 min)
- ✅ All needed packages available (SFML, Boost.Asio, GoogleTest)

---

## Conclusion

### Technology Stack Summary

| Layer | Technology | Key Reason |
|-------|-----------|------------|
| **Graphics** | SFML 2.6+ | **All 5 members proficient** → 0 days learning |
| **Networking** | Boost.Asio | Async I/O for multithreaded server |
| **Dependencies** | vcpkg | One-line CMake integration |

### Project Requirements Compliance

- ✅ **Binary UDP Protocol** → Boost.Asio provides low-level socket control
- ✅ **Multithreaded Server** → Boost.Asio io_context thread pool (non-blocking)
- ✅ **Cross-Platform (Linux/Windows)** → All technologies support both platforms seamlessly
- ✅ **Package Manager** → vcpkg with CMake integration
- ✅ **60 FPS Rendering** → SFML handles real-time 2D graphics

### Strategic Decisions

| Decision | Rationale | Impact |
|----------|-----------|--------|
| **SFML over alternatives** | Team already expert | Saves 3-14 days onboarding |
| **Boost.Asio over raw sockets** | Cross-platform async I/O | Eliminates platform-specific code |
| **vcpkg over Conan/FetchContent** | Simplest CMake integration | One-line configuration |

---

## References

### Official Documentation

- [SFML Documentation](https://www.sfml-dev.org/documentation/)
- [SFML Tutorials](https://www.sfml-dev.org/tutorials/)
- [Boost.Asio Documentation](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
- [CMake Documentation](https://cmake.org/documentation/)

### Comparative Studies

- [Game Networking Architectures](https://gafferongames.com/post/what_every_programmer_needs_to_know_about_game_networking/)
- [ECS vs OOP in Game Development](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/understanding-component-entity-systems-r3013/)
- [Binary vs Text Protocols](https://stackoverflow.com/questions/2645708/binary-vs-text-protocols)
- [UDP vs TCP for Games](https://gafferongames.com/post/udp_vs_tcp/)

### R-Type Project Documentation

- [RFC-0001: Engine Architecture](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/docs/rfcs/RFC-0001-engine-architecture.md)
- [README.md](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/README.md)
- [CONTRIBUTING.md](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/CONTRIBUTING.md)

---


### Official Documentation

- [SFML Documentation](https://www.sfml-dev.org/documentation/)
- [SFML Tutorials](https://www.sfml-dev.org/tutorials/)
- [Boost.Asio Documentation](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
- [CMake Documentation](https://cmake.org/documentation/)

### Comparative Studies

- [Game Networking Architectures](https://gafferongames.com/post/what_every_programmer_needs_to_know_about_game_networking/)
- [ECS vs OOP in Game Development](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/understanding-component-entity-systems-r3013/)
- [Binary vs Text Protocols](https://stackoverflow.com/questions/2645708/binary-vs-text-protocols)
- [UDP vs TCP for Games](https://gafferongames.com/post/udp_vs_tcp/)

### R-Type Project Documentation

- [README.md](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/README.md)
- [CONTRIBUTING.md](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/CONTRIBUTING.md)

---

*This document will be updated as technologies evolve and new requirements emerge in Part 2 of the project.*
