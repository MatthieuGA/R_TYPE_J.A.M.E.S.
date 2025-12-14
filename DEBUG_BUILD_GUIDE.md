# Debug Build Guide

## Overview

The project supports granular compile-time debug output control. You can enable specific debug categories without affecting performance in production builds.

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

### Selective Debug (Choose What You Want)
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

| Flag | Purpose | Output Example |
|------|---------|----------------|
| `DEBUG_PARTICLES` | Particle system rendering | `[DEBUG_PARTICLES] RenderParticles called with 150 particles` |
| `DEBUG_RENDERING` | General rendering operations | `[DEBUG_RENDERING] Texture loaded: player.png` |
| `DEBUG_NETWORK` | Network operations | `[DEBUG_NETWORK] Packet sent: 128 bytes` |

## Using in CMakeLists.txt

The debug flags are defined in the root `CMakeLists.txt`:

```cmake
option(DEBUG_PARTICLES "Enable particle rendering debug output" OFF)
option(DEBUG_RENDERING "Enable general rendering debug output" OFF)
option(DEBUG_NETWORK "Enable network debug output" OFF)
```

## Using in Code

### Including the Debug Header
```cpp
#include <debug/DebugConfig.hpp>
```

### Using Debug Macros
```cpp
// These compile to nothing if the flag is disabled (zero overhead)
DEBUG_PARTICLES_LOG("Particle count: " << count);
DEBUG_RENDERING_LOG("Drawing sprite at (" << x << ", " << y << ")");
DEBUG_NETWORK_LOG("Received packet of size " << size);
```

### Macro Expansion

**When Enabled:**
```cpp
DEBUG_PARTICLES_LOG("Test");
// Expands to:
std::cout << "[DEBUG_PARTICLES] " << "Test" << std::endl;
```

**When Disabled:**
```cpp
DEBUG_PARTICLES_LOG("Test");
// Expands to:
((void)0);  // Completely removed by optimizer
```

## Performance Impact

| Build Type | Debug Output | Performance Impact |
|------------|--------------|-------------------|
| Release (default) | OFF | **0%** (macros expand to no-op) |
| Debug (-DCMAKE_BUILD_TYPE=Debug) | ALL ON | ~2-5% (I/O overhead) |
| Selective (e.g., -DDEBUG_PARTICLES=ON) | Specific | <1% (minimal I/O) |

## IDE Integration

### VS Code (tasks.json)
```json
{
    "label": "CMake: Configure Debug with Particles",
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
Add CMake options in **Settings → Build, Execution, Deployment → CMake**:
```
-DDEBUG_PARTICLES=ON -DDEBUG_RENDERING=ON
```

## CI/CD Integration

### GitHub Actions Example
```yaml
- name: Build with Debug
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build -j$(nproc)

- name: Build Release (No Debug)
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)
```

## Adding New Debug Categories

1. **Add option in CMakeLists.txt:**
```cmake
option(DEBUG_PHYSICS "Enable physics debug output" OFF)

if(DEBUG_PHYSICS)
    add_compile_definitions(DEBUG_PHYSICS)
    message(STATUS "Physics debug output: ENABLED")
endif()
```

2. **Add macro in DebugConfig.hpp:**
```cpp
#ifdef DEBUG_PHYSICS
    #include <iostream>
    #define DEBUG_PHYSICS_LOG(msg) std::cout << "[DEBUG_PHYSICS] " << msg << std::endl
#else
    #define DEBUG_PHYSICS_LOG(msg) ((void)0)
#endif
```

3. **Use in code:**
```cpp
#include <debug/DebugConfig.hpp>

void PhysicsSystem::Update() {
    DEBUG_PHYSICS_LOG("Updating " << entities.size() << " physics entities");
}
```

## Troubleshooting

### Debug output not appearing
```bash
# Verify flags are set
cmake -S . -B build -DDEBUG_PARTICLES=ON
# Check CMake output for: "Particle debug output: ENABLED"

# Rebuild from scratch
rm -rf build
cmake -S . -B build -DDEBUG_PARTICLES=ON
cmake --build build
```

### Too much debug output
```bash
# Disable specific categories
cmake -S . -B build -DDEBUG_PARTICLES=OFF
cmake --build build

# Or switch to Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Best Practices

1. **Production Builds:** Always use `Release` build type with no debug flags
2. **Development:** Use selective debug flags (only what you're working on)
3. **CI/CD:** Test both Debug and Release builds
4. **Commit Code:** Never commit code with hardcoded debug output (use macros)
5. **Performance Testing:** Always benchmark with Release builds

## Summary

✅ **Zero overhead in production** (macros expand to no-op)  
✅ **Granular control** (enable only what you need)  
✅ **Easy to use** (simple CMake flags)  
✅ **Standard compliant** (conditional compilation, no runtime checks)  
✅ **IDE friendly** (works with all major IDEs)

---

**For more information, see:** `engine/include/debug/DebugConfig.hpp`
