# Plugin System

The R-TYPE J.A.M.E.S. engine features a dynamic plugin system that enables runtime loading of modular components without recompilation. This architecture provides flexibility, maintainability, and extensibility while adhering to industry best practices.

## Overview

The plugin system allows developers to:

- **Hot-swap implementations** at runtime without recompiling the main application
- **Extend functionality** by creating new plugin modules
- **Maintain loose coupling** between the engine core and specific implementations
- **Support multiple backends** for the same interface (e.g., SFML, OpenAL, custom audio engines)

## Architecture

The plugin system is built on three core components:

### 1. DLLoader - Dynamic Library Loader

A generic template class that handles the low-level details of loading shared libraries (`.so` files on Linux, `.dll` on Windows).

**Location:** `engine/include/loader/DLLoader.hpp`

**Key Features:**
- Type-safe dynamic library loading using C++ templates
- RAII-based resource management (automatic cleanup)
- Exception-based error handling
- Support for custom entry point symbols

### 2. Plugin Interfaces

Pure virtual interfaces that define the contract between the engine and plugins. These interfaces use abstract base classes with no implementation details.

**Key Principles:**
- All methods are pure virtual (`= 0`)
- No platform-specific dependencies in the interface
- Clear, well-documented API contracts
- Semantic versioning compatibility

### 3. Plugin Implementations

Concrete implementations of the interfaces, compiled as separate shared libraries. Each plugin:
- Implements all interface methods
- Provides a C-style entry point to avoid name mangling
- Returns instances via `std::shared_ptr` for automatic memory management
- Can be loaded/unloaded at runtime

## Current Plugins

### Audio Plugin System

**Interface:** `Engine::Audio::IAudioModule`
**Location:** `engine/include/audio/IAudioModule.hpp`
**Implementation:** SFML Audio Module (`client/plugins/audio/sfml/`)

The audio plugin system handles all sound effects and music playback. It provides:
- Sound effect loading and playback
- Music streaming with loop support
- Independent volume control for SFX and music
- Mute functionality
- Playback request system with categories

📘 **[Audio Plugin Development Guide](./audio-plugin-guide.md)**

### Video Plugin System

**Interface:** `Engine::Video::IVideoModule`
**Location:** `engine/include/video/IVideoModule.hpp`
**Rendering Layer:** `Engine::Rendering::RenderingEngine`
**Implementation:** SFML Video Module (`client/plugins/video/sfml/`)

The video plugin system uses a **3-layer architecture** for clean separation of concerns:

```
[Game Systems] → [RenderingEngine] → [IVideoModule Plugin] → [Graphics Library]
```

**RenderingEngine** provides game-level abstractions:
- High-level sprite and text rendering methods
- Automatic transform hierarchy handling
- Frame lifecycle management (BeginFrame/EndFrame)
- Camera system support (ready for future expansion)
- Centralized rendering logic

**IVideoModule Plugin** provides low-level graphics API:
- Window creation and event handling
- Primitive rendering operations (sprites, text, shapes)
- Resource loading (textures, fonts, shaders)
- Backend-agnostic types (Color, Vector2f, Transform)

**Key Features:**
- Plugin swapping at runtime via configuration
- Game systems decoupled from graphics details
- Simplified rendering code in systems (~90% less boilerplate)
- Support for multiple backends (SFML, SDL2, Raylib, etc.)

📘 **[Video Plugin Development Guide](./video-plugin-guide.md)**
📘 **[RenderingEngine Migration Guide](../../RENDERING_ENGINE_MIGRATION.md)**

## Plugin Development Guides

Comprehensive step-by-step guides for creating custom plugins:

- **[Audio Plugin Guide](./audio-plugin-guide.md)** - Create custom audio backends
- **[Video Plugin Guide](./video-plugin-guide.md)** - Create custom rendering backends
- **[Plugin Architecture](./architecture.md)** - Deep dive into the plugin system
- **[API Reference](./api-reference.md)** - Complete interface documentation

## Benefits

### For Developers

- **Rapid iteration:** Modify plugins without rebuilding the entire project
- **Isolated testing:** Test plugins independently from the main application
- **Flexible deployment:** Ship different plugin configurations for different platforms

### For the Project

- **Modularity:** Clear separation of concerns
- **Extensibility:** Easy to add new backends or features
- **Compliance:** Meets advanced project requirements for dynamic module loading

## Getting Started

To create your first plugin, follow our step-by-step guide:

👉 [Creating Your First Plugin](./audio-plugin-guide.md)

## Technical Requirements

- **C++23** standard compliance
- **CMake 3.23+** for build configuration
- **Linux:** Uses `dlopen`/`dlsym` (POSIX)
- **Windows:** Uses `LoadLibrary`/`GetProcAddress` (Win32 API)

## Best Practices

1. **Use RAII:** Let `DLLoader` handle library lifecycle
2. **Avoid global state:** Keep plugins stateless or properly initialized
3. **Document thoroughly:** All public API methods should have Doxygen comments
4. **Version compatibility:** Use semantic versioning for interfaces
5. **Error handling:** Provide clear error messages and graceful degradation

## Next Steps

- [Audio Plugin Development Guide](./audio-plugin-guide.md) - Complete tutorial
- [Plugin Architecture](./architecture.md) - In-depth technical details
- [API Reference](./api-reference.md) - Complete interface documentation
- [Troubleshooting](./troubleshooting.md) - Common issues and solutions
