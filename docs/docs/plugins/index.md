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

The audio subsystem demonstrates the plugin architecture with a complete implementation:

- **Interface:** `IAudioModule` - Defines audio backend operations
- **Implementation:** `SFMLAudioModule` - SFML-based audio backend
- **Adapter:** `PluginAudioBackend` - Bridges plugin interface to engine API

See the detailed guides:
- [Creating Audio Plugins](./audio-plugin-guide.md)
- [Plugin Architecture Deep Dive](./architecture.md)
- [API Reference](./api-reference.md)

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
