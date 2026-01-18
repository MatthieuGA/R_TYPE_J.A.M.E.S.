# Graphics Backend Plugin System

## Overview

The R-Type J.A.M.E.S. engine uses a **plugin architecture** to enable third-party graphics backends. This decouples the core engine from rendering implementation details, allowing developers to implement custom backends for SFML, OpenGL, Vulkan, SDL2, or other graphics libraries.

### Why C ABI?

Plugins use a **C Application Binary Interface (ABI)** for several critical reasons:

1. **ABI Stability**: C ABI is stable across compilers, compiler versions, and C++ standard versions. C++ ABIs (name mangling, exception handling) are not.
2. **Version Compatibility**: Old plugins compiled against old engine versions load safely into new engines without recompilation.
3. **Cross-Compiler Interop**: A plugin compiled with GCC can load into an engine compiled with Clang or MSVC.
4. **Isolation**: Plugins cannot accidentally use incompatible C++ features (RTTI, exceptions) that would break if both sides use different standard libraries.

### Supported Platforms

- **Linux**: `.so` libraries (GNU/Linux with glibc or musl)
- **Windows**: `.dll` libraries (Windows 7 and later)
- **macOS**: `.dylib` libraries (macOS 10.13 and later)

All platforms require a **C++20-compatible compiler** (GCC 12+, MSVC 2022 17.0+, Clang 15+).

---

## Plugin ABI Specification

### Required Entrypoints

All plugins **must** export these three C symbols. Use the `PLUGIN_EXPORT` macro (defined in `plugin_api.h`) to ensure correct visibility:

#### 1. `create_graphics_backend_v1`

```cpp
extern "C" PLUGIN_EXPORT PluginRenderContextHandle create_graphics_backend_v1(
    const char *config_json);
```

**Purpose**: Create and initialize a graphics backend instance.

**Parameters**:
- `config_json`: Optional JSON configuration string (may be `NULL`). Backend-specific options.
  - Example: `{"enable_vsync": true, "samples": 4}`
  - Parser is entirely backend-dependent.

**Returns**:
- Non-NULL `PluginRenderContextHandle` on success.
- `NULL` on failure (insufficient memory, invalid config, device not available, etc.).
  - Engine will log an error and fall back to another backend (or fail gracefully).

**Ownership**: Plugin allocates memory. Caller must eventually call `destroy_graphics_backend_v1()` to release.

**Threading**: Called once during engine initialization (single-threaded). No concurrent calls.

**Exception Safety**: May throw C++ exceptions. Loader catches and logs; engine continues safely.

**Contract Violation Consequences**: Returning an invalid pointer WILL cause a crash during rendering. Validate internal state thoroughly before returning.

---

#### 2. `destroy_graphics_backend_v1`

```cpp
extern "C" PLUGIN_EXPORT void destroy_graphics_backend_v1(
    PluginRenderContextHandle handle);
```

**Purpose**: Release resources allocated by `create_graphics_backend_v1()`.

**Parameters**:
- `handle`: Non-NULL handle returned by `create_graphics_backend_v1()`.
  - After this call, the handle is invalid and must not be used.

**Returns**: Nothing.

**Ownership**: Releases all resources associated with the handle.

**Threading**: Called once during engine shutdown (single-threaded).

**Exception Safety**: May throw. Loader logs but does not propagate exceptions.

**Contract Violation Consequences**:
- Failing to release resources → **Memory leak**.
- Double-freeing the same handle → **Heap corruption / crash**.
- Passing an invalid handle → **Segmentation fault**.

---

#### 3. `graphics_backend_name_v1`

```cpp
extern "C" PLUGIN_EXPORT const char *graphics_backend_name_v1(void);
```

**Purpose**: Return a human-readable name for the graphics backend (for logging and debugging).

**Returns**: Non-NULL pointer to a null-terminated C string. Examples:
- `"SFML 2.6.1 Graphics Backend"`
- `"OpenGL 4.6 Direct Rendering"`
- `"Vulkan 1.3 Graphics Backend"`

**Ownership**: Plugin owns the returned pointer. String must remain valid for the entire plugin lifetime.

**Threading**: Called once during plugin discovery (single-threaded).

**Exception Safety**: Should not throw.

---

### Handle Ownership and Lifetime

- **`PluginRenderContextHandle`** is an opaque `void*` pointer.
- It is actually a pointer to an `Engine::Graphics::IRenderContext` C++ object, cast to `void*` to maintain C ABI compliance.
- **Plugin owns the allocation and must manage cleanup in `destroy_graphics_backend_v1()`.**
- Engine wraps the handle in a `unique_ptr` with a custom deleter that calls `destroy_graphics_backend_v1()`.
- The engine **never calls delete or free on the handle directly**.

### Exception Handling Across C Boundary

C++ exceptions **may be thrown** in `create_graphics_backend_v1()`, but **must not propagate across the C boundary**:
- Exceptions are caught by the loader and logged to stderr.
- Engine continues with graceful fallback.
- If your plugin crashes during creation, the engine treats it as a failed backend.

**Rule**: Do not throw exceptions in `destroy_graphics_backend_v1()` or `graphics_backend_name_v1()`. If cleanup fails, log to stderr and continue.

### Threading Rules

- **All plugin calls are single-threaded** and happen from the main game loop thread.
- No concurrent calls to the same plugin instance.
- No concurrent calls from different game instances (current architecture).
- Plugins **do not need thread synchronization** for normal usage.
- If you need multithreading (e.g., async GPU uploads), use local mutexes, not global state.

---

## Naming & Discovery

### Plugin Library Names

The engine resolves `--graphics-backend=<name>` to a platform-specific filename:

| OS | Pattern | Example | Search Path |
|---|---|---|---|
| **Linux** | `libgraphics_<name>.so` | `libgraphics_custom.so` | `./plugins/` |
| **macOS** | `libgraphics_<name>.dylib` | `libgraphics_custom.dylib` | `./plugins/` |
| **Windows** | `graphics_<name>.dll` | `graphics_custom.dll` | `./plugins/` |

### Resolution Algorithm

When user specifies `--graphics-backend=custom`:

1. Check if backend is registered in static registry (e.g., "sfml" is always built-in).
2. If not found, construct filename: `libgraphics_custom.so` (on Linux).
3. Load from `./plugins/libgraphics_custom.so`.
4. Call `dlsym()` / `GetProcAddress()` to resolve the three entrypoints.
5. On success, register backend in factory for reuse.
6. On failure, fall back to default backend ("sfml") and log error.
7. If no backend is available, fatal error.

### How to Name Your Plugin

If creating a backend named "mybackend":

1. Create shared library: `libgraphics_mybackend.so` (Linux)
2. Distribute in directory: `./plugins/` (relative to executable)
3. User runs: `./r-type_client 127.0.0.1 50000 user --graphics-backend=mybackend`
4. Engine loads: `./plugins/libgraphics_mybackend.so`

**Recommendation**: Use lowercase, alphanumeric names without underscores or hyphens. Examples: `custom`, `opengl`, `vulkan`, `raylib`.

---

## Toolchain & ABI Constraints

### Why C ABI Matters for You

When you write a plugin, you are creating a **shared library with a C interface**. The engine (compiled separately, possibly with different tools) will:

1. Load your plugin at runtime.
2. Call C functions by name, using `dlsym()`.
3. Cast function pointers to the correct signature.
4. Call your code.

If your plugin and the engine disagree on memory layout, calling conventions, or function signatures, **the result is undefined behavior and likely a crash**.

### Compiler Compatibility Rules

**Supported Compilers**:
- GCC 12+ (Linux)
- Clang 15+ (Linux, macOS)
- MSVC 2022 (Windows)

**Constraints**:
- Plugin and engine **must both use C++20** standard (or higher).
- Do not mix C++17 and C++20 (ABI breaks).
- Do not mix debug and release builds (ABI breaks).
- Do not mix sanitizers (e.g., ASAN) between plugin and engine.
- Do not use compiler-specific extensions (-fno-exceptions, custom calling conventions).

**Recommendation**: Compile plugins with the same compiler, version, and flags as the engine.

### What Is NOT Allowed in ABI

The C ABI boundary separates your plugin from the engine. These C++ features **must not cross the boundary**:

| Feature | Why Not | Solution |
|---|---|---|
| **STL containers** (`std::vector`, `std::string`, `std::map`) | Allocation differences, iterator invalidation | Use C types: `char*`, pointers, arrays |
| **C++ exceptions** | Different exception handling implementations | Catch exceptions inside your code; return error codes to C caller |
| **RTTI / dynamic_cast** | Requires linked RTTI metadata | Use virtual functions or manual type checks |
| **Virtual functions across C boundary** | v-table layouts differ per compiler | Keep v-tables inside plugin only; don't expose virtual to C code |
| **Lambdas, templates** | Code generation varies | Instantiate templates in your compilation unit only |

**Safe Types Across Boundary**:
- `int`, `float`, `double`, `bool`
- `const char*` (null-terminated strings)
- `void*` (opaque handles, like `PluginRenderContextHandle`)
- Plain `struct` with no virtual functions or STL members
- Function pointers with C linkage

### Static Initialization Warnings

If your plugin uses global objects (e.g., `static std::vector`), you will see linker warnings about undefined behavior. **These are valid warnings**. Reasons:

1. **Initialization order is undefined** across shared library boundaries.
2. **Cleanup order is unpredictable** (reverse of initialization? undefined?).
3. **Thread safety of global construction is not guaranteed**.

**Best Practice**: Avoid global state. Move all initialization into `create_graphics_backend_v1()` and cleanup into `destroy_graphics_backend_v1()`.

---

## Building a Plugin

### File Structure

A minimal plugin looks like this:

```
my_graphics_plugin/
├── CMakeLists.txt           # Build configuration
├── src/
│   └── my_plugin.cpp        # Implementation (contains the three entrypoints)
├── include/
│   └── my_context.hpp       # Your IRenderContext implementation (optional)
└── README.md                # How to build and use
```

### Required Includes

```cpp
#include "graphics/IRenderContext.hpp"  // Interface definition
#include "graphics/plugin_api.h"        // C ABI, PLUGIN_EXPORT macro
```

Both headers are in the engine's `engine/include/graphics/` directory.

**Important**: Do NOT include:
- `client/` headers (client-specific code)
- `server/` headers (server-specific code)
- Other plugin implementations
- Game-logic components

### Linking Against Engine Headers

Plugins **only link against engine headers**, not the compiled engine library. The engine provides the interface (`IRenderContext`) as pure virtual classes; your plugin implements them.

**CMakeLists.txt example**:

```cmake
add_library(graphics_custom SHARED src/my_plugin.cpp)

# Add engine headers (read-only, no linking)
target_include_directories(graphics_custom PRIVATE
    ${ENGINE_ROOT}/engine/include
)

# Platform-specific settings
if(WIN32)
    set_target_properties(graphics_custom PROPERTIES
        PREFIX ""  # No "lib" prefix on Windows
        SUFFIX ".dll"
    )
else()
    set_target_properties(graphics_custom PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"  # or ".dylib" on macOS
    )
endif()
```

### External Dependencies (Graphics Libraries)

Your plugin **must** handle its own dependencies:

- **OpenGL plugin**: Link against OpenGL library (`-lGL` on Linux, `framework OpenGL` on macOS).
- **SDL2 plugin**: Link against SDL2 (use `find_package(SDL2)` or pkg-config).
- **Vulkan plugin**: Link against Vulkan loader library.
- **SFML plugin**: Link against SFML (see reference implementation in `plugins/graphics_sfml/`).

The engine does **not** ship or link these libraries. You provide them as part of your plugin package.

### Example Build Commands

**Linux (using CMake)**:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
ls lib/libgraphics_custom.so
```

**macOS**:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -GNinja
ninja
ls lib/libgraphics_custom.dylib
```

**Windows (MSVC)**:

```cmd
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build . --config Release
dir lib\graphics_custom.dll
```

---

## Packaging for Release

### Directory Layout

When distributing your plugin, use this layout:

```
graphics-custom-1.0/
├── README.md                     # Installation, usage, requirements
├── LICENSE                       # Software license
├── lib/
│   ├── libgraphics_custom.so    # Linux x86_64
│   ├── libgraphics_custom.dylib # macOS (universal or Intel/ARM variants)
│   └── graphics_custom.dll       # Windows x86_64
└── docs/
    └── IMPLEMENTATION_NOTES.md   # Your design choices
```

### Readme Template

Include a `README.md` in your package:

```markdown
# Custom Graphics Backend Plugin

## Requirements
- R-Type J.A.M.E.S. engine version X.Y.Z or later
- OpenGL 4.6+ (or your graphics library)
- 64-bit operating system

## Installation
1. Extract plugin files to `./plugins/` relative to the game executable.
2. Verify: `ls ./plugins/libgraphics_custom.so`
3. Run: `./r-type_client 127.0.0.1 50000 user --graphics-backend=custom`

## Supported Platforms
- Linux (glibc x86_64)
- macOS 10.13+ (Intel / Apple Silicon)
- Windows 10+ (x86_64)

## Performance Notes
[Describe performance characteristics, GPU requirements, known limitations]

## Troubleshooting
- "Plugin not found": Check filename matches `libgraphics_<name>.so`
- "Missing symbol": Verify ABI entrypoints are exported
- "Segfault during render": Check plugin handles invalid textures gracefully
```

### Versioning Recommendations

- **Engine Compatibility**: Document which engine versions your plugin supports.
  - Example: "Compatible with R-Type J.A.M.E.S. 1.0 and later."
- **Plugin Version**: Include version in package name and documentation.
  - Example: `graphics-custom-1.0.0` for semver compliance.
- **ABI Version**: Current ABI is v1 (symbols are `*_v1`). If engine adds v2, old v1 plugins remain compatible.

### Backward Compatibility Expectations

- **Engine promises**: v1 plugins will load into any future engine version.
- **Plugin promises**: If you update your plugin, maintain the v1 ABI signature. Do not change function signatures.
- **Breaking changes**: If you must break ABI (e.g., new required symbol), ship as v2 plugin alongside v1.

---

## Debugging & Validation

### Checking Exported Symbols

**Linux / macOS** (using `nm`):

```bash
nm -D libgraphics_custom.so | grep graphics_backend
# Expected output:
# 0000000000001234 T create_graphics_backend_v1
# 0000000000001256 T destroy_graphics_backend_v1
# 00000000000012ab T graphics_backend_name_v1
```

**Windows** (using `dumpbin`):

```cmd
dumpbin /EXPORTS graphics_custom.dll | findstr graphics_backend
# Expected output:
#      1  0x1234 create_graphics_backend_v1
#      2  0x1256 destroy_graphics_backend_v1
#      3  0x12ab graphics_backend_name_v1
```

### Common Failure Modes

| Symptom | Cause | Solution |
|---|---|---|
| **Plugin not found** | File doesn't exist in `./plugins/` or wrong filename | Check filename: `libgraphics_<name>.so` (Linux), `graphics_<name>.dll` (Windows) |
| **Missing symbol error** | Entrypoint not exported (missing `PLUGIN_EXPORT`) | Add `PLUGIN_EXPORT` to all three functions; rebuild |
| **Segfault on load** | Static initialization crash (global ctors) | Move all state init to `create_graphics_backend_v1()` |
| **Segfault on render** | Returned invalid handle or uninitialized object | Validate plugin state before returning from `create()` |
| **Link error (undefined reference)** | Plugin links against incompatible library version | Rebuild plugin with same compiler/flags as engine |
| **"Invalid config_json"** | Plugin rejects configuration | Check JSON syntax; see plugin's README for valid options |
| **Texture not loaded** | Path format differs from expected | Plugins use `.` relative to executable for asset paths |

### Typical Loader Error Messages

When the engine loads your plugin, you may see messages like:

```
[GraphicsPluginLoader] Loading plugin: ./plugins/libgraphics_custom.so
[GraphicsPluginLoader] Successfully loaded backend: custom
```

On failure:

```
[GraphicsPluginLoader] Error: Failed to load library: ./plugins/libgraphics_custom.so: cannot open shared object file
[GraphicsPluginLoader] Error: Missing symbol: create_graphics_backend_v1
[GraphicsPluginLoader] Falling back to default backend: sfml
```

These errors are **not fatal** — the engine will use a fallback. Check your plugin build and distribution.

---

## Testing Your Plugin

### Manual Testing

1. Build your plugin: `cmake --build . && cp lib/libgraphics_custom.so ../../plugins/`
2. Run engine: `./r-type_client 127.0.0.1 50000 user --graphics-backend=custom`
3. If backend loads, engine proceeds. If it fails, it falls back to SFML.
4. Check stderr for loader messages.

### Automated Testing

The engine includes a plugin loader test suite (`tests/unit/test_dl_loader.cpp`) that verifies:

- Plugins load from correct paths
- Entrypoints are resolved
- Handles are valid
- Destroy doesn't crash

Write similar tests for your plugin to verify it works across platforms.

### Pixel-Perfect Testing

The engine includes pixel-comparison tests that render a known scene and compare output to a baseline image. Use this to verify your plugin produces pixel-identical output to reference (if your goal is compatibility) or to detect regressions in your implementation.

---

## Reference Implementation

The SFML graphics backend plugin (`plugins/graphics_sfml/sfml_plugin.cpp`) is a complete, working example. Study it to understand:

1. How to implement `IRenderContext` (pure virtual interface)
2. How to export the three ABI entrypoints
3. How to handle configuration
4. How to manage resources (textures, fonts)

Use it as a template for your own plugin.

---

## Limitations & Future Work

### Current Constraints (v1)

- **No hot-reload**: Plugins are loaded once and kept loaded. To replace a plugin, restart the engine.
- **No unloading**: `dlclose()` / `FreeLibrary()` is never called. Plugins remain in memory for engine lifetime.
- **Single window**: All rendering goes to one window. Multi-window plugins are not supported yet.
- **Synchronous rendering**: No asynchronous texture upload or GPU operations (all calls are synchronous).
- **No plugin discovery**: You must specify a backend name. No auto-scanning of `./plugins/` directory.

### Future Enhancements (Speculative, Not Committed)

- Plugin discovery (auto-scan directory, list available backends)
- Configuration files (`.toml`, `.json` per backend)
- Hot-reload capability (unload old, load new)
- Extended ABI (v2) with new features (e.g., GPU command buffer recording)
- Plugin marketplace (registry of known backends)

**None of these are planned or guaranteed.**

---

## Summary

To create a graphics backend plugin:

1. **Implement** `IRenderContext` (the pure virtual interface).
2. **Export** the three C ABI entrypoints (`create_v1`, `destroy_v1`, `name_v1`) with `PLUGIN_EXPORT`.
3. **Build** as a shared library with the correct platform-specific name.
4. **Place** in `./plugins/` directory.
5. **Test** by running with `--graphics-backend=<name>`.
6. **Document** your plugin's requirements, options, and limitations.
7. **Package** with all dependencies and a clear README.

The C ABI is stable, simple, and proven. Your plugin will work across compiler versions, architectures, and future engine versions—as long as you follow the contract.

Good luck!
