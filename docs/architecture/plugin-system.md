# Plugin Architecture Overview

## Introduction

The R-Type J.A.M.E.S. engine uses a **plugin-based architecture** for rendering and audio backends. This design provides flexibility, testability, and the ability to swap implementations without changing game code.

## Architecture Layers

```
┌─────────────────────────────────────────────────────────┐
│                    Game Code                            │
│              (ECS Systems, GameWorld)                   │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                 RenderingEngine                         │
│          (High-level game abstractions)                 │
│   Camera, Particles, Resources, Z-ordering             │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                  IVideoModule                           │
│           (Backend-agnostic interface)                  │
│   Initialize, DrawSprite, LoadTexture, etc.            │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│              Concrete Implementation                    │
│        SFMLVideoModule / SDLVideoModule / etc.         │
└─────────────────────────────────────────────────────────┘
```

## Why Plugin Architecture?

### 1. **Backend Flexibility**
- Swap rendering backends (SFML → SDL → Vulkan) without changing game code
- Choose best backend per platform (SFML on desktop, SDL on mobile)
- Test with mock plugins without GPU dependencies

### 2. **Testability**
- Unit tests can use mock video plugins (no window, no GPU)
- CI/CD runs on headless servers
- Faster test execution (no actual rendering)

### 3. **Maintainability**
- Clear separation of concerns
- Backend-specific code isolated in plugins
- Game logic independent of rendering implementation

### 4. **Future-Proofing**
- Easy to add new backends (Raylib, custom Vulkan, etc.)
- API changes don't affect game code
- Multiple backends can coexist

## Current Implementations

### Video Plugins
| Plugin | Status | Platform | Notes |
|--------|--------|----------|-------|
| **SFMLVideoModule** | ✅ Production | Desktop | Default, well-tested |
| **SDLVideoModule** | 🔜 Planned | Desktop/Mobile | Future cross-platform |
| **MockVideoModule** | 🔜 Planned | Testing | Headless, for CI/CD |
| **VulkanVideoModule** | 💭 Future | Desktop | High-performance |

### Audio Plugins
| Plugin | Status | Platform | Notes |
|--------|--------|----------|-------|
| **SFMLAudioModule** | ✅ Production | Desktop | Default backend |
| **PluginAudioBackend** | ✅ Production | Desktop | Plugin wrapper |

## Key Components

### IVideoModule Interface
**Location:** `engine/include/video/IVideoModule.hpp`

The core interface that all video backends must implement:

```cpp
class IVideoModule {
public:
    // Lifecycle
    virtual bool Initialize(unsigned int width, unsigned int height, 
                           const std::string& title) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;
    
    // Rendering
    virtual void DrawSprite(const std::string& texture_id, 
                           const Transform& transform, 
                           const IntRect* source_rect = nullptr) = 0;
    virtual void DrawText(const std::string& text, 
                         const std::string& font_id,
                         const Transform& transform) = 0;
    
    // Resources
    virtual bool LoadTexture(const std::string& id, 
                            const std::string& path) = 0;
    virtual bool UnloadTexture(const std::string& id) = 0;
    
    // ... more methods
};
```

### RenderingEngine
**Location:** `engine/include/rendering/RenderingEngine.hpp`

High-level game rendering API built on top of plugins:

```cpp
class RenderingEngine {
public:
    RenderingEngine(std::shared_ptr<Video::IVideoModule> plugin);
    
    // Game-level features
    void RenderSprite(const std::string& texture_id, 
                     const Vector2f& position, 
                     float rotation, const Vector2f& scale);
    void RenderParticles(const std::vector<Vector2f>& particles,
                        const std::vector<Color>& colors);
    
    // Camera system
    Camera& GetCamera();
    
    // Statistics
    const RenderStats& GetStats() const;
};
```

### SFMLVideoModule
**Location:** `client/plugins/video/sfml/SFMLVideoModule.hpp`

SFML implementation of the video interface:

```cpp
class SFMLVideoModule : public IVideoModule {
private:
    sf::RenderWindow window_;
    std::unordered_map<std::string, sf::Texture> textures_;
    std::unordered_map<std::string, sf::Font> fonts_;
    // ... SFML-specific resources
};
```

## Usage Example

### Basic Setup

```cpp
#include <video/IVideoModule.hpp>
#include <rendering/RenderingEngine.hpp>
#include <loader/DLLoader.hpp>

// Load video plugin
Engine::DLLoader<Engine::Video::IVideoModule> loader;
loader.open("plugins/libsfml_video_plugin.so");
auto video_module = loader.getInstance("entryPoint");

// Create rendering engine
auto rendering_engine = std::make_unique<Engine::Rendering::RenderingEngine>(
    video_module);

// Initialize
if (!rendering_engine->Initialize(1920, 1080, "My Game")) {
    throw std::runtime_error("Failed to initialize rendering");
}

// Game loop
while (rendering_engine->IsWindowOpen()) {
    rendering_engine->BeginFrame(Color(0, 0, 0, 255));
    
    // Your game rendering here
    rendering_engine->RenderSprite("player", 
        Vector2f(100, 100), 0.0f, Vector2f(1, 1));
    
    rendering_engine->EndFrame();
}

rendering_engine->Shutdown();
```

## Backend-Agnostic Types

All types used in the interface are backend-agnostic:

```cpp
namespace Engine::Graphics {
    struct Color { uint8_t r, g, b, a; };
    struct Vector2f { float x, y; };
    struct IntRect { int left, top, width, height; };
    struct FloatRect { float left, top, width, height; };
}
```

**No SFML types leak into game code!**

## Plugin Loading

### Dynamic Loading (Runtime)
```cpp
// Load plugin at runtime
Engine::DLLoader<Engine::Video::IVideoModule> loader;
loader.open(config.GetVideoBackend()); // From config file
auto module = loader.getInstance("entryPoint");
```

### Static Linking (Compile-time)
```cpp
// Use specific implementation directly
auto module = std::make_shared<SFMLVideoModule>();
```

## Thread Safety

⚠️ **IMPORTANT:** Video plugins are **NOT thread-safe** by default.

- All rendering calls must be from the **same thread**
- Do not call plugin methods from update/physics threads
- Resource loading should be done on the render thread

```cpp
// ❌ WRONG - calling from update thread
void UpdateSystem() {
    rendering_engine->LoadTexture("enemy", "enemy.png"); // BAD!
}

// ✅ CORRECT - loading during initialization
void Initialize() {
    rendering_engine->LoadTexture("enemy", "enemy.png"); // GOOD!
}

// ✅ CORRECT - all rendering on main thread
void RenderLoop() {
    rendering_engine->BeginFrame();
    rendering_engine->RenderSprite("enemy", pos, rot, scale);
    rendering_engine->EndFrame();
}
```

## Benefits Summary

| Benefit | Description |
|---------|-------------|
| **Flexibility** | Swap backends without code changes |
| **Testability** | Mock plugins for unit tests |
| **Performance** | Choose optimal backend per platform |
| **Maintenance** | Isolated backend code |
| **Future-proof** | Easy to add new backends |
| **Portability** | Platform-specific implementations |

## Future Possibilities

### Planned Backends
- **SDL2/SDL3** - Better cross-platform support, mobile-friendly
- **Raylib** - Simple, lightweight, educational
- **Custom Vulkan** - Maximum performance control
- **WebGPU** - Web browser support

### Planned Features
- Plugin hot-reloading (swap backend at runtime)
- Multi-window support
- VR/AR backends
- Offscreen rendering backends (for recording/streaming)

## Related Documentation

- [IVideoModule API Reference](../api-reference/ivideo-module.md)
- [RenderingEngine API Reference](../api-reference/rendering-engine.md)
- [Creating a Video Plugin](../guides/creating-video-plugin.md)
- [Resource Management](../guides/resource-management.md)
- [Thread Safety](./thread-safety.md)

---

**Next Steps:**
- Read the [RenderingEngine API guide](../guides/using-rendering-engine.md) to learn how to use the rendering API
- See [Plugin Implementation Guide](../guides/creating-video-plugin.md) to create your own backend
- Check [Debug System](../guides/debug-system.md) for debugging tips
