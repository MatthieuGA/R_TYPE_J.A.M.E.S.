# Debug System Guide

## Overview

The engine provides a **compile-time debug system** with zero overhead in production builds. Debug output is controlled via CMake flags, allowing you to enable specific debug categories without performance impact.

## Quick Start

### Production Build (No Debug)
```bash
# Zero debug overhead, maximum performance
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Development Build (All Debug)
```bash
# All debug categories enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Selective Debug
```bash
# Enable only particle debugging
cmake -S . -B build -DDEBUG_PARTICLES=ON
cmake --build build -j$(nproc)

# Enable multiple categories
cmake -S . -B build \
    -DDEBUG_PARTICLES=ON \
    -DDEBUG_RENDERING=ON \
    -DDEBUG_NETWORK=ON
cmake --build build -j$(nproc)
```

## Debug Categories

| Flag | Purpose | Output Prefix |
|------|---------|---------------|
| `DEBUG_PARTICLES` | Particle system rendering | `[DEBUG_PARTICLES]` |
| `DEBUG_RENDERING` | General rendering operations | `[DEBUG_RENDERING]` |
| `DEBUG_NETWORK` | Network packets and connections | `[DEBUG_NETWORK]` |

### DEBUG_PARTICLES

Tracks particle system operations:

```cpp
[DEBUG_PARTICLES] RenderParticles called with 150 particles
[DEBUG_PARTICLES] Creating vertex array for 150 particles
[DEBUG_PARTICLES] Reserved 600 vertices
[DEBUG_PARTICLES] Built 600 vertices
[DEBUG_PARTICLES] Calling DrawVertices with 600 vertices
[DEBUG_PARTICLES] DrawVertices completed
```

**Use when:**
- Debugging particle rendering issues
- Optimizing particle batch sizes
- Verifying particle system updates

### DEBUG_RENDERING

Tracks general rendering operations:

```cpp
[DEBUG_RENDERING] Texture loaded: player.png (ID: player_sprite)
[DEBUG_RENDERING] Font loaded: dogica.ttf (ID: ui_font)
[DEBUG_RENDERING] Shader loaded: wave.frag (ID: water_shader)
[DEBUG_RENDERING] Drawing 1523 sprites this frame
```

**Use when:**
- Debugging resource loading
- Tracking draw calls
- Profiling rendering performance

### DEBUG_NETWORK

Tracks network operations:

```cpp
[DEBUG_NETWORK] TCP connection established
[DEBUG_NETWORK] Sending CONNECT_REQ (44 bytes, username: Player1)
[DEBUG_NETWORK] Received UDP packet: 128 bytes
[DEBUG_NETWORK] Snapshot received: 15 entities
```

**Use when:**
- Debugging connection issues
- Tracking packet flow
- Analyzing network performance

## Using Debug Macros in Code

### 1. Include the Debug Header

```cpp
#include <debug/DebugConfig.hpp>
```

### 2. Use Debug Macros

```cpp
void RenderSystem() {
    DEBUG_RENDERING_LOG("Rendering " << entity_count << " entities");
    
    for (auto entity : entities) {
        DEBUG_RENDERING_LOG("Drawing entity " << entity.id 
                          << " at (" << entity.x << ", " << entity.y << ")");
    }
}

void ParticleSystem() {
    DEBUG_PARTICLES_LOG("Updating " << particles.size() << " particles");
    DEBUG_PARTICLES_LOG("Particle pool size: " << pool.capacity());
}

void NetworkSystem() {
    DEBUG_NETWORK_LOG("Sending input packet: " << packet.size() << " bytes");
}
```

### 3. Macro Expansion

**When DEBUG_PARTICLES=ON:**
```cpp
DEBUG_PARTICLES_LOG("Test " << value);
// Expands to:
std::cout << "[DEBUG_PARTICLES] " << "Test " << value << std::endl;
```

**When DEBUG_PARTICLES=OFF (default):**
```cpp
DEBUG_PARTICLES_LOG("Test " << value);
// Expands to:
((void)0);  // Completely optimized away by compiler
```

## Performance Impact

| Build Configuration | Debug Output | Performance Impact | Use Case |
|---------------------|--------------|-------------------|----------|
| **Release** (default) | None | 0% | Production, benchmarks |
| **Release + DEBUG_PARTICLES** | Particles only | <0.5% | Particle debugging |
| **Release + DEBUG_RENDERING** | Rendering only | <1% | Render profiling |
| **Debug** (all flags) | All categories | 2-5% | Development |

## CMake Configuration

### Root CMakeLists.txt

```cmake
# Debug output options
option(DEBUG_PARTICLES "Enable particle rendering debug output" OFF)
option(DEBUG_RENDERING "Enable general rendering debug output" OFF)
option(DEBUG_NETWORK "Enable network debug output" OFF)

# Pass debug flags to compiler
if(DEBUG_PARTICLES)
    add_compile_definitions(DEBUG_PARTICLES)
    message(STATUS "Particle debug output: ENABLED")
endif()

# Enable all debug in Debug build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(CMAKE_BUILD_TYPE_DEBUG)
    message(STATUS "Debug build: All debug outputs ENABLED")
endif()
```

### Verification

After running CMake, check for confirmation messages:

```bash
$ cmake -S . -B build -DDEBUG_PARTICLES=ON
-- Particle debug output: ENABLED
-- Configuring done
```

## IDE Integration

### VS Code (tasks.json)

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: Configure with Particle Debug",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-S", ".",
                "-B", "build",
                "-DDEBUG_PARTICLES=ON",
                "-DCMAKE_BUILD_TYPE=Debug"
            ]
        },
        {
            "label": "CMake: Configure Production",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-S", ".",
                "-B", "build",
                "-DCMAKE_BUILD_TYPE=Release"
            ]
        }
    ]
}
```

### CLion

Add CMake options in **Settings → Build, Execution, Deployment → CMake**:

**Debug Profile:**
```
-DCMAKE_BUILD_TYPE=Debug
```

**Custom Debug Profile (Selective):**
```
-DDEBUG_PARTICLES=ON -DDEBUG_RENDERING=ON -DCMAKE_BUILD_TYPE=Release
```

## Adding New Debug Categories

### 1. Add CMake Option

Edit `CMakeLists.txt`:

```cmake
option(DEBUG_PHYSICS "Enable physics debug output" OFF)

if(DEBUG_PHYSICS)
    add_compile_definitions(DEBUG_PHYSICS)
    message(STATUS "Physics debug output: ENABLED")
endif()
```

### 2. Add Macro Definition

Edit `engine/include/debug/DebugConfig.hpp`:

```cpp
#ifdef DEBUG_PHYSICS
    #include <iostream>
    #define DEBUG_PHYSICS_LOG(msg) \
        std::cout << "[DEBUG_PHYSICS] " << msg << std::endl
#else
    #define DEBUG_PHYSICS_LOG(msg) ((void)0)
#endif
```

### 3. Use in Code

```cpp
#include <debug/DebugConfig.hpp>

void PhysicsSystem::Update(float dt) {
    DEBUG_PHYSICS_LOG("Updating " << rigid_bodies.size() << " bodies");
    
    for (auto& body : rigid_bodies) {
        DEBUG_PHYSICS_LOG("Body " << body.id 
                        << " velocity: (" << body.vx << ", " << body.vy << ")");
    }
}
```

## Best Practices

### DO ✅

```cpp
// Use debug macros for development-only output
DEBUG_RENDERING_LOG("Loaded texture: " << path);

// Include variable values for debugging
DEBUG_PARTICLES_LOG("Batch size: " << batch.size() 
                  << ", capacity: " << batch.capacity());

// Track function entry/exit
DEBUG_NETWORK_LOG("Entering ProcessPacket()");
// ... code ...
DEBUG_NETWORK_LOG("Exiting ProcessPacket()");
```

### DON'T ❌

```cpp
// ❌ Don't use raw std::cout in production code
std::cout << "Rendering " << count << " sprites" << std::endl;

// ❌ Don't use debug macros for user-facing messages
DEBUG_RENDERING_LOG("Game started!"); // Wrong - use normal logging

// ❌ Don't use debug macros for error messages
DEBUG_NETWORK_LOG("ERROR: Connection failed"); // Wrong - use std::cerr

// ❌ Don't do expensive operations in debug macros
DEBUG_PARTICLES_LOG("Dump: " << ExpensiveSerialize(data)); // BAD!
```

## Conditional Logic

If you need conditional logic based on debug state:

```cpp
#ifdef DEBUG_PARTICLES
    // This code only exists when DEBUG_PARTICLES is ON
    std::vector<ParticleDebugInfo> debug_info;
    for (auto& p : particles) {
        debug_info.push_back(p.GetDebugInfo());
    }
    DEBUG_PARTICLES_LOG("Debug info collected: " << debug_info.size());
#endif
```

## Troubleshooting

### Debug Output Not Appearing

**Problem:** Built with `-DDEBUG_PARTICLES=ON` but no output.

**Solution:**
```bash
# 1. Clean rebuild
rm -rf build
cmake -S . -B build -DDEBUG_PARTICLES=ON
cmake --build build

# 2. Verify flag is set
cmake -S . -B build -DDEBUG_PARTICLES=ON | grep "Particle debug"
# Should show: "-- Particle debug output: ENABLED"

# 3. Check binary has the flag
strings build/client/r-type_client | grep DEBUG_PARTICLES
```

### Too Much Debug Output

**Problem:** Console flooded with debug messages.

**Solution:**
```bash
# Disable specific categories
cmake -S . -B build -DDEBUG_PARTICLES=OFF

# Or switch to Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug Output in Production

**Problem:** Accidentally shipped with debug enabled.

**Solution:**
```bash
# Always use Release for production
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Verify no debug flags
cmake -L build | grep DEBUG
# Should show all OFF
```

## CI/CD Integration

### GitHub Actions Example

```yaml
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      # Test with debug for detailed logs
      - name: Configure (Debug)
        run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
      
      - name: Build
        run: cmake --build build -j$(nproc)
      
      - name: Test with Debug Output
        run: cd build && ctest --output-on-failure
  
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      # Production build with no debug
      - name: Configure (Release)
        run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
      
      - name: Build
        run: cmake --build build -j$(nproc)
      
      - name: Package
        run: # ... packaging steps
```

## Summary

✅ **Zero overhead** in production (macros expand to no-op)  
✅ **Granular control** (enable only what you need)  
✅ **Easy to use** (simple CMake flags)  
✅ **Standard compliant** (conditional compilation)  
✅ **IDE friendly** (works with all major IDEs)  
✅ **CI/CD ready** (test with debug, ship without)

---

**Related Documentation:**
- [Plugin System](../architecture/plugin-system.md)
- [RenderingEngine API](../guides/using-rendering-engine.md)
- [Performance Profiling](./performance-profiling.md)
