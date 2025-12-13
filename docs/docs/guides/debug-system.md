# Debug Build System

Complete guide to the R-TYPE J.A.M.E.S. debug build system with compile-time debug output control.

## Overview

The project supports **granular compile-time debug output control** with zero performance overhead in production builds. You can enable specific debug categories without affecting runtime performance.

## Quick Start

### Production Build (No Debug Output)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Full Debug Build (All Debug Output)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Selective Debug (Choose Specific Categories)
```bash
# Enable only particle rendering debug
cmake -S . -B build -DDEBUG_PARTICLES=ON
cmake --build build -j$(nproc)

# Enable only network debug
cmake -S . -B build -DDEBUG_NETWORK=ON
cmake --build build -j$(nproc)

# Enable multiple categories
cmake -S . -B build -DDEBUG_PARTICLES=ON -DDEBUG_RENDERING=ON
cmake --build build -j$(nproc)
```

## Available Debug Categories

| CMake Flag | Purpose | Output Prefix | Performance Impact |
|------------|---------|---------------|-------------------|
| `DEBUG_PARTICLES` | Particle system rendering details | `[DEBUG_PARTICLES]` | ~1% (when enabled) |
| `DEBUG_RENDERING` | General rendering operations | `[DEBUG_RENDERING]` | ~1% (when enabled) |
| `DEBUG_NETWORK` | Network packet details | `[DEBUG_NETWORK]` | ~2% (when enabled) |

:::tip
**In production builds** (Release mode, no flags), debug macros expand to `((void)0)` and are completely removed by the compiler optimizer - **zero overhead**.
:::

## How It Works

### 1. CMake Configuration

Debug flags are defined in the root `CMakeLists.txt`:

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

### 2. Debug Macros (DebugConfig.hpp)

**Location:** `engine/include/debug/DebugConfig.hpp`

```cpp
// Particle rendering debug output
#ifdef DEBUG_PARTICLES
    #include <iostream>
    #define DEBUG_PARTICLES_LOG(msg) \
        std::cout << "[DEBUG_PARTICLES] " << msg << std::endl
#else
    #define DEBUG_PARTICLES_LOG(msg) ((void)0)
#endif

// General rendering debug output
#ifdef DEBUG_RENDERING
    #include <iostream>
    #define DEBUG_RENDERING_LOG(msg) \
        std::cout << "[DEBUG_RENDERING] " << msg << std::endl
#else
    #define DEBUG_RENDERING_LOG(msg) ((void)0)
#endif

// Network debug output
#ifdef DEBUG_NETWORK
    #include <iostream>
    #define DEBUG_NETWORK_LOG(msg) \
        std::cout << "[DEBUG_NETWORK] " << msg << std::endl
#else
    #define DEBUG_NETWORK_LOG(msg) ((void)0)
#endif
```

### 3. Usage in Code

```cpp
#include <debug/DebugConfig.hpp>

void RenderingEngine::RenderParticles(const std::vector<Vector2f> &particles) {
    DEBUG_PARTICLES_LOG("RenderParticles called with " << particles.size() << " particles");
    
    // ... rendering code ...
    
    DEBUG_PARTICLES_LOG("Built " << vertices.size() << " vertices");
    
    // These macros compile to NOTHING if DEBUG_PARTICLES is not defined
}
```

## Macro Expansion Examples

### When DEBUG_PARTICLES is Enabled:
```cpp
DEBUG_PARTICLES_LOG("Particle count: " << count);

// Expands to:
std::cout << "[DEBUG_PARTICLES] " << "Particle count: " << count << std::endl;
```

### When DEBUG_PARTICLES is Disabled:
```cpp
DEBUG_PARTICLES_LOG("Particle count: " << count);

// Expands to:
((void)0);  // Completely removed by compiler optimizer
```

## Performance Impact

| Build Configuration | Debug Output | Typical Overhead |
|---------------------|--------------|------------------|
| **Release** (default, no flags) | OFF | **0%** - macros expand to no-op |
| **Release + DEBUG_PARTICLES=ON** | Particles only | &lt;1% - minimal I/O overhead |
| **Debug** (CMAKE_BUILD_TYPE=Debug) | All categories | 2-5% - I/O and buffering overhead |

:::info
The overhead comes from console I/O operations, not from checking flags at runtime. When disabled, debug statements have **zero runtime cost**.
:::

## IDE Integration

### VS Code

Add to `.vscode/tasks.json`:

```json
{
    "label": "CMake: Configure with Debug Particles",
    "type": "shell",
    "command": "cmake",
    "args": [
        "-S", ".",
        "-B", "build",
        "-DDEBUG_PARTICLES=ON",
        "-DCMAKE_BUILD_TYPE=Debug"
    ]
}
```

### CLion

Go to **Settings → Build, Execution, Deployment → CMake** and add CMake options:

```
-DDEBUG_PARTICLES=ON -DDEBUG_RENDERING=ON
```

### Visual Studio Code with CMake Tools Extension

Add to `.vscode/settings.json`:

```json
{
    "cmake.configureSettings": {
        "DEBUG_PARTICLES": "ON",
        "DEBUG_RENDERING": "ON"
    }
}
```

## CI/CD Integration

### GitHub Actions Example

```yaml
- name: Build Debug with Particles
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build -j$(nproc)

- name: Build Release (No Debug)
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)
```

### Testing Different Configurations

```bash
#!/bin/bash
# test_builds.sh - Test all debug configurations

configs=(
    ""  # Release, no debug
    "-DDEBUG_PARTICLES=ON"
    "-DDEBUG_RENDERING=ON"
    "-DDEBUG_NETWORK=ON"
    "-DCMAKE_BUILD_TYPE=Debug"  # All debug
)

for config in "${configs[@]}"; do
    echo "Testing configuration: $config"
    rm -rf build
    cmake -S . -B build $config
    cmake --build build -j$(nproc)
    ./build/tests/engine_tests
done
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
    DEBUG_PHYSICS_LOG("Updating " << entities.size() << " physics entities");
    
    for (auto &entity : entities) {
        DEBUG_PHYSICS_LOG("Entity " << entity.id << " velocity: " 
                         << entity.velocity.x << ", " << entity.velocity.y);
    }
}
```

### 4. Enable During Build

```bash
cmake -S . -B build -DDEBUG_PHYSICS=ON
cmake --build build
```

## Example Output

### With DEBUG_PARTICLES Enabled:

```
[DEBUG_PARTICLES] RenderParticles called with 150 particles
[DEBUG_PARTICLES] Creating vertex array for 150 particles
[DEBUG_PARTICLES] Reserved 600 vertices
[DEBUG_PARTICLES] Built 600 vertices
[DEBUG_PARTICLES] Calling DrawVertices with 600 vertices
[DEBUG_PARTICLES] DrawVertices completed
```

### Without DEBUG_PARTICLES (Production):

```
(No output - macros expand to nothing)
```

## Troubleshooting

### Debug Output Not Appearing

1. **Verify CMake configuration:**
   ```bash
   cmake -S . -B build -DDEBUG_PARTICLES=ON
   # Check for: "Particle debug output: ENABLED"
   ```

2. **Rebuild from scratch:**
   ```bash
   rm -rf build
   cmake -S . -B build -DDEBUG_PARTICLES=ON
   cmake --build build
   ```

3. **Check if header is included:**
   ```cpp
   #include <debug/DebugConfig.hpp>  // Required!
   ```

### Too Much Debug Output

**Disable specific categories:**
```bash
cmake -S . -B build -DDEBUG_PARTICLES=OFF
cmake --build build
```

**Or switch to Release build:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Performance Testing

Always benchmark with **Release builds** and **no debug flags**:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/client/r-type_client
```

## Best Practices

### ✅ DO:
- Use debug macros for development-time diagnostics
- Enable only the categories you're actively debugging
- Test both Debug and Release builds in CI/CD
- Remove or disable debug output before performance profiling
- Use descriptive messages with context

### ❌ DON'T:
- Commit code with hardcoded `std::cout` debug output
- Enable all debug categories in production
- Rely on debug output for critical application logic
- Include sensitive information (passwords, keys) in debug output
- Use debug macros in performance-critical tight loops

## Summary

✅ **Zero overhead in production** - macros expand to no-op  
✅ **Granular control** - enable only what you need  
✅ **Easy to use** - simple CMake flags  
✅ **Standard compliant** - conditional compilation, no runtime checks  
✅ **IDE friendly** - works with all major IDEs  
✅ **Extensible** - easy to add new debug categories  

For implementation details, see [`engine/include/debug/DebugConfig.hpp`](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/engine/include/debug/DebugConfig.hpp).
