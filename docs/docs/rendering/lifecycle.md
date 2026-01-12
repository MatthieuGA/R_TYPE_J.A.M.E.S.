# Rendering Initialization and Lifecycle

## Overview

The rendering system initialization follows a specific sequence to ensure all resources are loaded and component metadata is configured before the game loop begins. This document describes the startup process and runtime flow.

## Startup Sequence (Initialization Phase)

### Phase 1: Create the SFML Window

```cpp
sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type J.A.M.E.S.");
window.setFramerateLimit(60);  // 60 FPS cap
```

**Responsibility:** Main client application (e.g., `client/main.cpp`).

### Phase 2: Instantiate SFMLRenderContext

```cpp
SFMLRenderContext render_context(window);
```

**Responsibility:** Main client application.

**Notes:**
- The context holds a reference to the window, not ownership.
- The window must remain valid for the lifetime of the render context.

### Phase 3: Load Resources

```cpp
// Textures
render_context.LoadTexture("player", "assets/player.png");
render_context.LoadTexture("enemy_wave_1", "assets/enemies/wave1.png");
render_context.LoadTexture("projectile", "assets/projectile.png");
render_context.LoadTexture("particles", "assets/particles.png");

// Fonts
render_context.LoadFont("default", "assets/fonts/arial.ttf");
render_context.LoadFont("ui", "assets/fonts/ui_bold.ttf");

// Shaders (if any)
render_context.LoadShader("charged_glow", 
  "assets/shaders/charged.vert", "assets/shaders/charged.frag");
```

**Responsibility:** `InitRegistrySystems` (discussed below) or a dedicated resource loader.

**Timing:** After window creation but before spawning game entities.

**Error Handling:** If a resource fails to load, the backend logs a warning and continues. Entities using that resource may not render.

### Phase 4: Initialize the ECS Registry and Systems

```cpp
auto registry = CreateRegistry();  // Create empty registry

// Register and run initialization systems
InitializeDrawableStaticSystem(registry, render_context);
InitializeAnimationSystem(registry, render_context);

// Register update systems (run every frame)
registry.RegisterSystem<AnimationSystem>();
registry.RegisterSystem<DrawableSystem>(render_context);
registry.RegisterSystem<DrawTextSystem>(render_context);
registry.RegisterSystem<ParticleEmitterSystem>(render_context);
```

**Location:** This is typically wrapped in `InitRegistrySystems()` function in the client code.

**Order:** Critical—initialization systems must run before any game logic that spawns entities.

**Dependency:** Initialization systems require the render context to query resource properties (e.g., texture size).

### Phase 5: Spawn Initial Game Entities

After initialization systems, the client spawns the player, enemies, UI elements, etc.

```cpp
// Example: Spawn player
auto player = registry.SpawnEntity();
registry.AddComponent<Transform>(player, {{960, 540}, 0, {1, 1}});
registry.AddComponent<Drawable>(player, {"player", {0, 0}, 0, 0, {1, 1}, true, ""});
registry.AddComponent<AnimationFrame>(player, {
  .current_frame_ = 0,
  .elapsed_time_ = 0,
  .frame_duration_ = 0.1f,
  .animation_mode_ = "strip",
  .frame_count_ = 4,
  .frame_width_ = 64,
  .grid_cols_ = 1
});

// At this point:
// - InitializeDrawableStaticSystem has already run (skipped animated entities).
// - InitializeAnimationSystem will process this entity on next system update.
```

**Timing:** During scene loading (main menu, level start).

## Runtime Flow (Game Loop)

### Each Frame Execution Order

```cpp
while (window.isOpen()) {
  // Frame timing
  float delta_time = clock.restart().asSeconds();
  
  // 1. Handle input and update game logic
  HandleInput(registry);
  UpdateGameLogic(registry, delta_time);
  
  // 2. Update animation frames
  AnimationSystem(registry, delta_time);
  
  // 3. Render
  render_context.Clear(sf::Color(20, 20, 30));  // Dark background
  
  DrawableSystem(registry, render_context, delta_time);
  DrawTextSystem(registry, render_context, delta_time);
  ParticleEmitterSystem(registry, render_context, delta_time);
  
  render_context.Display();  // Present frame to screen
  
  // 4. Cleanup (remove dead entities, etc.)
  CleanupDeadEntities(registry);
}
```

### Key Points

1. **Animation updates first:** `AnimationSystem` calculates the current frame before rendering.
2. **All renders before Display:** All draw calls happen before `Display()`.
3. **Single render context:** All systems share the same `IRenderContext` instance.
4. **Delta time:** Passed to systems for frame-rate-independent animation and movement.

## Registry System Initialization (InitRegistrySystems)

### Function Signature

```cpp
void InitRegistrySystems(Registry& registry,
                         IRenderContext& render_context);
```

**Location:** Typically in `client/engine/InitRegistrySystems.cpp` or similar.

### Implementation Example

```cpp
void InitRegistrySystems(Registry& registry,
                         IRenderContext& render_context) {
  // === Resource Loading ===
  auto sfml_context = dynamic_cast<SFMLRenderContext*>(&render_context);
  if (sfml_context) {
    // Load all textures
    sfml_context->LoadTexture("player", "assets/sprites/player.png");
    sfml_context->LoadTexture("enemy_wave_1", "assets/sprites/enemies/wave1.png");
    sfml_context->LoadTexture("projectile", "assets/sprites/projectile.png");
    sfml_context->LoadTexture("particles", "assets/sprites/particles.png");
    
    // Load all fonts
    sfml_context->LoadFont("default", "assets/fonts/arial.ttf");
    sfml_context->LoadFont("ui", "assets/fonts/ui_bold.ttf");
    
    // Load all shaders
    sfml_context->LoadShader("charged_glow",
      "assets/shaders/charged.vert", "assets/shaders/charged.frag");
  }
  
  // === Initialize Static Drawable Components ===
  InitializeDrawableStaticSystem(registry, render_context);
  
  // === Initialize Animated Drawable Components ===
  InitializeAnimationSystem(registry, render_context);
  
  // === Other systems (input, physics, networking, etc.) ===
  // ... register other systems as needed
}
```

**Key Decision:** Resource loading is coupled with `InitRegistrySystems`. To decouple:
1. Create a separate `LoadResources(SFMLRenderContext&)` function.
2. Call it explicitly before `InitRegistrySystems`.

## Component Setup Timeline

### For a Static Sprite (e.g., Background)

```cpp
// Creation
auto background = registry.SpawnEntity();
registry.AddComponent<Transform>(background, {{0, 0}, 0, {1, 1}});
registry.AddComponent<Drawable>(background, {"background", {0, 0}, 0, 0, {1, 1}, true, ""});

// Initialization (happens once at startup)
// InitializeDrawableStaticSystem:
//   1. Checks if entity has AnimationFrame → No
//   2. Queries texture size: {1920, 1080}
//   3. Sets origin: {960, 540}

// Runtime (every frame)
// DrawableSystem:
//   1. Checks visibility: true
//   2. Renders sprite at position with computed origin
```

### For an Animated Sprite (e.g., Player)

```cpp
// Creation
auto player = registry.SpawnEntity();
registry.AddComponent<Transform>(player, {{960, 540}, 0, {1, 1}});
registry.AddComponent<Drawable>(player, {"player", {0, 0}, 0, 0, {1, 1}, true, ""});
registry.AddComponent<AnimationFrame>(player, {
  .current_frame_ = 0,
  .elapsed_time_ = 0,
  .frame_duration_ = 0.1f,
  .animation_mode_ = "strip",
  .frame_count_ = 4,
  .frame_width_ = 64,
  .grid_cols_ = 1
});

// Initialization (happens once at startup)
// InitializeDrawableStaticSystem:
//   1. Checks if entity has AnimationFrame → Yes
//   2. Skips this entity

// InitializeAnimationSystem:
//   1. Queries texture size: {256, 64}  (4 frames × 64px wide, 64px tall)
//   2. Calculates frame_height: 64 / 4 = 16 (no, that's wrong; height is full texture)
//   3. Actually: frame_height = 64 (texture is 256 wide, 64 tall; 4 frames in strip)
//   4. Sets origin: {32, 32}  (center of each frame)

// Runtime (every frame)
// AnimationSystem:
//   1. Increments elapsed_time_ by delta_time
//   2. If elapsed_time_ >= 0.1f:
//      a. Advances current_frame_: 0 → 1 → 2 → 3 → 0 (loops)
//      b. Resets elapsed_time_
//   3. Updates drawable.frame_ with current_frame_

// DrawableSystem:
//   1. Checks visibility: true
//   2. If AnimationFrame exists: calculate source_rect from current_frame_
//      - frame_height = 64 (full height of strip)
//      - Frame 0: {0, 0, 64, 64}
//      - Frame 1: {64, 0, 64, 64}
//      - Frame 2: {128, 0, 64, 64}
//      - Frame 3: {192, 0, 64, 64}
//   3. Renders sprite with calculated source_rect
```

## Camera and View Management

### Default View (World Space)

By default, the view is aligned with the window:
- Top-left: (0, 0)
- Bottom-right: (window_width, window_height)

### Custom View (Camera Follow)

To implement camera follow:

```cpp
void UpdateCameraSystem(Registry& registry, IRenderContext& render_context) {
  auto camera_view = registry.View<CameraFollow, Transform>();
  
  if (camera_view.size() > 0) {
    auto target = *camera_view.begin();
    auto& target_transform = registry.GetComponent<Transform>(target);
    
    // Set camera center to follow target
    render_context.SetView(sf::View(
      target_transform.position_,
      {1920, 1080}  // View size (can be larger/smaller for zoom)
    ));
  }
}
```

**Timing:** Call before rendering systems so all draws use the updated view.

## Handling Dynamic Entity Creation

Entities can be spawned during the game loop (e.g., projectiles, enemies):

```cpp
// In a projectile system
auto projectile = registry.SpawnEntity();
registry.AddComponent<Transform>(projectile, {pos, 0, {1, 1}});
registry.AddComponent<Drawable>(projectile, {"projectile", {0, 0}, 0, 0, {1, 1}, true, ""});

// No AnimationFrame, so InitializeDrawableStaticSystem will process it next startup.
// But it's spawned during gameplay, so we manually initialize:
auto texture_size = render_context.GetTextureSize("projectile");
registry.GetComponent<Drawable>(projectile).origin_ = texture_size / 2.0f;
```

**Recommendation:** For dynamic entities, compute origin immediately after spawning instead of waiting for initialization systems.

## Deferred Initialization (Advanced)

For large games with many assets, lazy-loading may be preferred:

```cpp
class LazyRenderContext : public IRenderContext {
 private:
  std::map<std::string, std::string> texture_paths_;  // Key → path mapping
  mutable std::unordered_map<std::string, sf::Texture> texture_cache_;
  
 public:
  void RegisterTexture(const std::string& key, const std::string& path) {
    texture_paths_[key] = path;
  }
  
  void DrawSprite(const DrawSpriteParams& params) override {
    // Load on-demand if not in cache
    if (texture_cache_.find(params.texture_key) == texture_cache_.end()) {
      auto it = texture_paths_.find(params.texture_key);
      if (it != texture_paths_.end()) {
        LoadTexture(it->second);
      }
    }
    // ... rest of DrawSprite
  }
};
```

**Trade-off:** Reduces startup time but may cause frame drops when new assets are first used.

## Performance Considerations

### Batch Rendering

All draws to the same render context are queued and submitted in order. To optimize:
1. Sort draw calls by texture (to reduce state changes).
2. Use texture atlases (multiple sprites on one texture).
3. Render particles in a single draw call per emitter.

### Resource Caching

Resources are cached in `SFMLRenderContext`:
- **Textures:** Loaded once, reused across all draws.
- **Fonts:** Loaded once; glyph cache is managed by SFML.
- **Shaders:** Loaded once, reused across all draws.

Avoid reloading resources unless the file changes.

### Delta Time Precision

Use `float delta_time` (seconds) for all timing:
```cpp
auto elapsed = clock.restart().asSeconds();  // Returns float
AnimationSystem(registry, elapsed);  // Frame-rate independent
```

## Teardown (Shutdown)

When exiting the game:

```cpp
// 1. Stop the game loop
window.close();

// 2. Cleanup entities
registry.Clear();  // Remove all entities

// 3. Destroy render context (implicit via RAII)
// (SFMLRenderContext destructor will clean up cached resources)

// 4. Destroy window (implicit via RAII)
// (window goes out of scope, closes automatically)
```

No explicit cleanup needed due to RAII; destructors handle resource deallocation.

## Debugging and Logging

### Enable Verbose Logging

Set log level to debug during development:
```cpp
Logger::SetLevel(LogLevel::Debug);

// Now initialization systems will log:
// [DEBUG] Loading texture 'player' from 'assets/player.png'
// [DEBUG] Initializing drawable for entity 42 (texture: player, origin: 960, 540)
```

### Check Resource Status

Query the render context for loaded resources (custom debug method):
```cpp
render_context.DebugLogLoadedResources();
// Output:
// Textures: player (256x256), enemy_wave_1 (512x512)
// Fonts: default (Arial 12)
// Shaders: charged_glow (compiled)
```

## References

- [Rendering Overview](./overview.md)
- [IRenderContext API](./irendercontext.md)
- [SFML Backend](./sfml-backend.md)
- [Rendering Systems](./systems.md)
- [Architecture](../architecture.md)
