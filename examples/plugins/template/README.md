# Template Graphics Backend Plugin

This directory contains a minimal, fully-documented template for building a graphics backend plugin for R-Type J.A.M.E.S.

## Overview

This template demonstrates:

- ✅ How to implement `IRenderContext` (the pure virtual interface)
- ✅ How to export the three C ABI entrypoints (`create_v1`, `destroy_v1`, `name_v1`)
- ✅ Proper CMake configuration for cross-platform builds
- ✅ Exception safety and error handling
- ✅ Memory management across the C boundary
- ✅ How to avoid common pitfalls (global state, static initialization, etc.)

The code is **heavily commented** to explain the "why" behind each design choice.

## Before You Start

Read `../../docs/plugins.md` for the complete plugin system documentation, including:

- Plugin ABI specification
- Naming conventions
- Toolchain constraints
- Packaging recommendations
- Debugging tips

**This template assumes you've read the main documentation.**

## Getting Started

### 1. Copy and Rename

```bash
# Copy this template to a new directory
cp -r examples/plugins/template examples/plugins/mybackend

# Rename all occurrences of "template" to your backend name (e.g., "custom", "opengl")
cd examples/plugins/mybackend
sed -i 's/template/mybackend/g' CMakeLists.txt src/template_plugin.cpp
mv src/template_plugin.cpp src/mybackend_plugin.cpp
```

### 2. Implement Your Rendering

Edit `src/mybackend_plugin.cpp` and replace the dummy `TemplateRenderContext` with your real implementation:

```cpp
class MyBackendRenderContext : public Engine::Graphics::IRenderContext {
public:
  MyBackendRenderContext(const char* config_json) {
    // TODO: Create window/surface
    // TODO: Parse config_json if needed
    // TODO: Initialize GPU state
    // TODO: Load default shaders/textures
  }

  bool BeginFrame() override {
    // TODO: Clear surface, reset state, start GPU command recording
    return true;
  }

  bool EndFrame() override {
    // TODO: Flush commands, present to window
    return true;
  }

  bool RenderSprite(const Engine::Graphics::DrawableSprite& sprite) override {
    // TODO: Load/cache texture, apply transform, render
    return true;
  }

  // ... implement other methods ...
};
```

### 3. Handle Dependencies

If your plugin uses external libraries (OpenGL, Vulkan, SDL2, etc.), add them to CMakeLists.txt:

```cmake
# Example for OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(graphics_mybackend PRIVATE OpenGL::GL)

# Example for SDL2
find_package(SDL2 REQUIRED)
target_include_directories(graphics_mybackend PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(graphics_mybackend PRIVATE ${SDL2_LIBRARIES})
```

### 4. Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Plugin should be at: build/libgraphics_mybackend.so (Linux)
```

### 5. Test

```bash
# Copy plugin to game's plugins directory
mkdir -p ../../build/plugins
cp libgraphics_mybackend.so ../../build/plugins/

# Run the game with your backend
cd ../../build
./client/r-type_client 127.0.0.1 50000 testuser --graphics-backend=mybackend
```

## Key Design Patterns

### Pattern 1: Configuration via JSON

The `config_json` parameter lets you pass options to your plugin:

```cpp
// In create_graphics_backend_v1():
if (config_json != nullptr) {
    // Parse JSON (use a simple parser, e.g., nlohmann/json, or hand-parse)
    // Example: {"enable_vsync": true, "samples": 4}
    std::string cfg(config_json);
    // Extract and apply settings
}
```

### Pattern 2: Resource Caching

Textures and shaders should be cached to avoid reloading:

```cpp
// In RenderSprite():
if (texture_cache_.find(sprite.texture_path) == texture_cache_.end()) {
    // Load texture from file
    auto texture = LoadTextureFromFile(sprite.texture_path);
    texture_cache_[sprite.texture_path] = texture;
}
auto texture = texture_cache_[sprite.texture_path];
// Render with texture
```

### Pattern 3: Error Handling

Always handle errors gracefully:

```cpp
bool MyContext::RenderSprite(const DrawableSprite& sprite) {
    if (sprite.texture_path == nullptr) {
        std::cerr << "Error: texture_path is NULL" << std::endl;
        return false;
    }
    
    if (sprite.texture_path[0] == '\0') {
        std::cerr << "Error: texture_path is empty" << std::endl;
        return false;
    }
    
    // Render...
    return true;
}
```

### Pattern 4: Logging

Use `std::cout` and `std::cerr` for debugging. The game logs these to stderr:

```cpp
std::cout << "[MyBackend] Rendering sprite at (" << x << ", " << y << ")" << std::endl;
std::cerr << "[MyBackend] Warning: texture not found: " << path << std::endl;
```

## Common Mistakes to Avoid

### ❌ Mistake 1: Global State

```cpp
// DON'T do this:
static GLuint default_shader = 0;  // Undefined, dangerous
static std::vector<Texture> cache;  // Global init order problems
```

**✅ Instead**: Store state in `MyBackendRenderContext`:

```cpp
class MyBackendRenderContext : public IRenderContext {
private:
    GLuint default_shader_;
    std::map<std::string, Texture> texture_cache_;
};
```

### ❌ Mistake 2: Using STL at ABI Boundary

```cpp
// DON'T return std::vector across ABI:
extern "C" std::vector<int> some_function();  // WRONG!
```

**✅ Instead**: Use C types:

```cpp
extern "C" const int* some_function(int* out_count) {
    *out_count = 5;
    static int array[5] = {1,2,3,4,5};
    return array;
}
```

### ❌ Mistake 3: Not Exporting Symbols

```cpp
// DON'T forget PLUGIN_EXPORT:
extern "C" void my_function() { }  // May not be visible!
```

**✅ Always use**:

```cpp
extern "C" PLUGIN_EXPORT void my_function() { }
```

### ❌ Mistake 4: Not Handling NULL Pointers

```cpp
bool RenderSprite(const DrawableSprite& sprite) {
    // DON'T assume pointers are valid:
    std::string path = sprite.texture_path;  // sprite.texture_path might be NULL!
}
```

**✅ Always check**:

```cpp
bool RenderSprite(const DrawableSprite& sprite) {
    if (sprite.texture_path == nullptr) return false;
    std::string path = sprite.texture_path;
}
```

### ❌ Mistake 5: Throwing Exceptions in Destroy

```cpp
extern "C" void destroy_graphics_backend_v1(Handle h) {
    if (cleanup_failed) {
        throw std::runtime_error("Cleanup failed");  // DON'T!
    }
}
```

**✅ Instead, log and continue**:

```cpp
extern "C" void destroy_graphics_backend_v1(Handle h) {
    if (cleanup_failed) {
        std::cerr << "Cleanup warning: ..." << std::endl;
        return;
    }
}
```

## Checking Your Plugin

### Verify Symbols Are Exported

```bash
# Linux/macOS
nm -D build/libgraphics_mybackend.so | grep graphics_backend

# Expected output:
# 0000000000001234 T create_graphics_backend_v1
# 0000000000001256 T destroy_graphics_backend_v1
# 00000000000012ab T graphics_backend_name_v1
```

### Check Symbol Visibility (Linux)

```bash
objdump -t build/libgraphics_mybackend.so | grep graphics_backend
# Should show ".g" (global) visibility for all three symbols
```

### Debug Plugin Loading

Run the game with verbose output:

```bash
./r-type_client 127.0.0.1 50000 user --graphics-backend=mybackend 2>&1 | grep -i "plugin\|backend"
```

You should see:

```
[GraphicsPluginLoader] Loading plugin: ./plugins/libgraphics_mybackend.so
[GraphicsPluginLoader] Successfully loaded backend: mybackend
[Template Plugin] create_graphics_backend_v1 called
[MyBackendRenderContext] Created with config: {}
[MyBackendRenderContext::BeginFrame]
[MyBackendRenderContext::RenderSprite] texture=...
...
```

## Performance Considerations

1. **Caching**: Load textures and shaders once, reuse them.
2. **Batching**: Group similar draw calls to reduce GPU state changes.
3. **Async Loading**: Consider async texture/shader loading (if architecture supports it).
4. **Profiling**: Use GPU profilers (NVIDIA Nsight, AMD Radeon GPU Profiler) to find bottlenecks.

## Packaging Your Plugin

When distributing:

1. Build for all target platforms (Linux, macOS, Windows).
2. Create a directory: `graphics-mybackend-1.0/`
3. Include:
   - `lib/libgraphics_mybackend.so` (Linux)
   - `lib/libgraphics_mybackend.dylib` (macOS)
   - `lib/graphics_mybackend.dll` (Windows)
   - `README.md` (your plugin documentation)
   - `LICENSE` (software license)
4. Document:
   - System requirements (GPU, driver versions)
   - Configuration options
   - Known limitations
   - Performance characteristics

See `../../docs/plugins.md` section "Packaging for Release" for details.

## Next Steps

1. **Read the full plugin documentation**: `../../docs/plugins.md`
2. **Study the reference SFML plugin**: `../graphics_sfml/sfml_plugin.cpp`
3. **Check the plugin loader tests**: `../../tests/unit/test_dl_loader.cpp`
4. **Test locally** with `--graphics-backend=mybackend`
5. **Debug with symbol inspection** (nm, objdump, dumpbin)
6. **Package and distribute**

## Support

For questions or issues:

1. Check `../../docs/plugins.md` for detailed ABI specification
2. Review `../graphics_sfml/` for a working reference implementation
3. Read comments in `src/template_plugin.cpp` for design rationale
4. Examine `../../engine/include/graphics/IRenderContext.hpp` for interface details

Good luck! Your plugin will work across compiler versions and future engine releases—as long as you follow the C ABI contract.
