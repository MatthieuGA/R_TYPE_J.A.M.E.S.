# Rendering Engine API

Complete guide to using the RenderingEngine for game development in R-TYPE J.A.M.E.S.

## Overview

The **RenderingEngine** is a high-level rendering abstraction that sits between your game systems and the low-level video plugin. It provides game-oriented methods for rendering sprites, text, particles, and managing resources.

```
[Your Game Systems] → [RenderingEngine] → [IVideoModule Plugin] → [SFML/SDL/etc]
```

**Key Benefits:**
- Simplified API - no need to manually manage textures, transforms, or draw calls
- Plugin-agnostic - works with any IVideoModule implementation
- Resource management - automatic texture reference counting
- Camera support - built-in coordinate system transformations
- Performance tracking - render statistics for optimization

## Getting Started

### Initialization

```cpp
#include <rendering/RenderingEngine.hpp>
#include <video/IVideoModule.hpp>

// 1. Load video plugin
Engine::DLLoader<Engine::Video::IVideoModule> loader;
loader.open("lib/sfml_video_module.so");
auto video_plugin = loader.getInstance("entryPoint");

// 2. Create rendering engine
auto rendering_engine = std::make_unique<Engine::Rendering::RenderingEngine>(video_plugin);

// 3. Initialize with window parameters
if (!rendering_engine->Initialize(1920, 1080, "My Game")) {
    throw std::runtime_error("Failed to initialize rendering engine");
}

// 4. Verify initialization
if (!rendering_engine->IsInitialized()) {
    throw std::runtime_error("Rendering engine not properly initialized");
}
```

### Basic Frame Loop

```cpp
while (rendering_engine->IsWindowOpen()) {
    // Handle events
    Engine::Video::Event event;
    while (rendering_engine->PollEvent(event)) {
        if (event.type == Engine::Video::EventType::CLOSED) {
            rendering_engine->CloseWindow();
        }
    }
    
    // Begin frame with clear color
    rendering_engine->BeginFrame(Engine::Graphics::Color(30, 30, 80, 255));
    
    // Render your game here...
    RenderSprites();
    RenderText();
    RenderParticles();
    
    // Present frame to screen
    rendering_engine->EndFrame();
}

// Cleanup
rendering_engine->Shutdown();
```

## Core Methods

### Frame Lifecycle

#### `BeginFrame(const Color &clear_color)`

Starts a new frame and clears the screen with the specified color.

```cpp
// Dark blue background
rendering_engine->BeginFrame(Engine::Graphics::Color(30, 30, 80, 255));

// Black background
rendering_engine->BeginFrame(Engine::Graphics::Color::Black);
```

:::danger Critical
**Must be called** before any rendering operations. Throws exception if plugin is null.
:::

#### `EndFrame()`

Presents the rendered frame to the screen.

```cpp
rendering_engine->EndFrame();
```

:::danger Critical
**Must be called** after all rendering operations. Throws exception if plugin is null.
:::

### Window Management

#### `IsWindowOpen() const`

Checks if the window is still open.

```cpp
while (rendering_engine->IsWindowOpen()) {
    // Game loop...
}
```

#### `CloseWindow()`

Closes the window (typically called in response to close event).

```cpp
if (event.type == Engine::Video::EventType::CLOSED) {
    rendering_engine->CloseWindow();
}
```

#### `GetWindowSize() const`

Returns the current window dimensions.

```cpp
Engine::Graphics::Vector2f size = rendering_engine->GetWindowSize();
std::cout << "Window: " << size.x << "x" << size.y << std::endl;
```

#### `SetWindowTitle(const std::string &title)`

Changes the window title at runtime.

```cpp
rendering_engine->SetWindowTitle("R-TYPE - Level 3");
```

### Event Handling

#### `PollEvent(Event &event)`

Retrieves the next event from the queue.

```cpp
Engine::Video::Event event;
while (rendering_engine->PollEvent(event)) {
    switch (event.type) {
        case Engine::Video::EventType::CLOSED:
            rendering_engine->CloseWindow();
            break;
        case Engine::Video::EventType::KEY_PRESSED:
            if (event.data.key_code == 57) {  // Space key
                player->Shoot();
            }
            break;
        case Engine::Video::EventType::MOUSE_MOVED:
            mouse_x = event.data.mouse_x;
            mouse_y = event.data.mouse_y;
            break;
    }
}
```

## Resource Management

### Textures

#### `LoadTexture(const std::string &id, const std::string &path)`

Loads a texture with reference counting.

```cpp
// Load texture
bool success = rendering_engine->LoadTexture("player", "assets/player.png");
if (!success) {
    std::cerr << "Failed to load player texture" << std::endl;
}

// Load the same texture again - no actual load, just increments ref count
rendering_engine->LoadTexture("player", "assets/player.png");
```

:::tip Reference Counting
The same texture can be loaded multiple times (ref counted). The actual GPU resource is only loaded once and only freed when all references are released.
:::

#### `UnloadTexture(const std::string &id)`

Decrements reference count and unloads when it reaches zero.

```cpp
// Decrement ref count
rendering_engine->UnloadTexture("player");

// When ref count reaches 0, GPU memory is freed
rendering_engine->UnloadTexture("player");
```

#### `GetTextureSize(const std::string &id) const`

Returns the dimensions of a loaded texture.

```cpp
auto size = rendering_engine->GetTextureSize("player");
std::cout << "Texture size: " << size.x << "x" << size.y << std::endl;
```

### Fonts

#### `LoadFont(const std::string &id, const std::string &path)`

Loads a font for text rendering.

```cpp
if (!rendering_engine->LoadFont("main_font", "assets/dogica.ttf")) {
    std::cerr << "Failed to load font" << std::endl;
}
```

#### `UnloadFont(const std::string &id)`

Unloads a previously loaded font.

```cpp
rendering_engine->UnloadFont("main_font");
```

#### `GetTextBounds(const std::string &text, const std::string &font_id, unsigned int character_size)`

Calculates the bounding box of text before rendering.

```cpp
auto bounds = rendering_engine->GetTextBounds(
    "Game Over", 
    "main_font", 
    48
);

// Center text on screen
float x = (window_width - bounds.width) / 2.0f;
float y = (window_height - bounds.height) / 2.0f;
```

### Shaders

#### `LoadShader(const std::string &id, const std::string &vertex_path, const std::string &fragment_path)`

Loads a shader program.

```cpp
bool success = rendering_engine->LoadShader(
    "wave_shader",
    "shaders/wave.vert",
    "shaders/wave.frag"
);
```

#### `UnloadShader(const std::string &id)`

Unloads a shader program.

```cpp
rendering_engine->UnloadShader("wave_shader");
```

#### `SetShaderParameter(const std::string &shader_id, const std::string &name, float value)`

Sets a uniform parameter in a shader.

```cpp
// Animate wave effect
float time = total_time_clock.GetElapsedTime().AsSeconds();
rendering_engine->SetShaderParameter("wave_shader", "time", time);
rendering_engine->SetShaderParameter("wave_shader", "amplitude", 10.0f);
```

## Rendering Methods

### Sprites

#### `RenderSprite(const std::string &texture_id, const Transform &transform, const IntRect *texture_rect, const Color &color, int z_index)`

Renders a single sprite.

```cpp
Engine::Video::Transform transform;
transform.position = {100.0f, 200.0f};
transform.rotation = 45.0f;  // degrees
transform.scale = {2.0f, 2.0f};
transform.origin = {16.0f, 16.0f};  // Center of 32x32 sprite

// Render full texture
rendering_engine->RenderSprite("player", transform, nullptr, Engine::Graphics::Color::White, 1);

// Render sprite sheet frame
Engine::Graphics::IntRect frame(0, 0, 32, 32);
rendering_engine->RenderSprite("spritesheet", transform, &frame, Engine::Graphics::Color::White, 1);
```

**Parameters:**
- `texture_id` - ID of loaded texture
- `transform` - Position, rotation, scale, origin
- `texture_rect` - Optional: sub-rectangle for sprite sheets (nullptr = full texture)
- `color` - Tint color (White = no tint)
- `z_index` - Rendering layer (higher = drawn on top)

### Text

#### `RenderText(const std::string &text, const std::string &font_id, const Transform &transform, unsigned int character_size, const Color &color, int z_index)`

Renders text.

```cpp
Engine::Video::Transform transform;
transform.position = {400.0f, 300.0f};

rendering_engine->RenderText(
    "Score: 12345",
    "main_font",
    transform,
    24,  // character size
    Engine::Graphics::Color::White,
    10  // z_index (render on top)
);
```

### Particles

#### `RenderParticles(const std::vector<Vector2f> &particles, const std::vector<Color> &colors, const std::vector<float> &sizes, int z_index)`

Renders a batch of particles efficiently.

```cpp
std::vector<Engine::Graphics::Vector2f> positions;
std::vector<Engine::Graphics::Color> colors;
std::vector<float> sizes;

// Create 100 particles
for (int i = 0; i < 100; ++i) {
    positions.push_back({x + rand_offset(), y + rand_offset()});
    colors.push_back(Engine::Graphics::Color(255, 128, 0, 200));  // Orange
    sizes.push_back(4.0f);
}

// Render all particles in one draw call
rendering_engine->RenderParticles(positions, colors, sizes, 0);
```

:::tip Performance
Particles are batched into a single draw call using vertex arrays. Much more efficient than rendering individual sprites.
:::

### Primitives

#### `DrawRectangle(const FloatRect &rect, const Color &color, const Color *outline_color, float outline_thickness)`

Draws a filled rectangle with optional outline.

```cpp
// Health bar background
Engine::Graphics::FloatRect bg_rect(50, 50, 200, 20);
rendering_engine->DrawRectangle(
    bg_rect,
    Engine::Graphics::Color(50, 50, 50, 255),  // Dark gray fill
    &Engine::Graphics::Color::White,  // White outline
    2.0f  // 2px outline
);

// Health bar fill
Engine::Graphics::FloatRect health_rect(50, 50, health_percent * 200, 20);
rendering_engine->DrawRectangle(
    health_rect,
    Engine::Graphics::Color(0, 255, 0, 255),  // Green
    nullptr,  // No outline
    0.0f
);
```

#### `DrawCircle(const Vector2f &center, float radius, const Color &color, const Color *outline_color, float outline_thickness)`

Draws a filled circle with optional outline.

```cpp
// Explosion effect
rendering_engine->DrawCircle(
    {explosion_x, explosion_y},
    explosion_radius,
    Engine::Graphics::Color(255, 100, 0, 128),  // Semi-transparent orange
    &Engine::Graphics::Color(255, 200, 0, 255),  // Yellow outline
    3.0f
);
```

## Camera System

### Camera Structure

```cpp
struct Camera {
    Vector2f position{0.0f, 0.0f};  // Camera position in world space
    float zoom = 1.0f;               // Zoom level (1.0 = normal, 2.0 = 2x zoom)
    Vector2f size{1920.0f, 1080.0f}; // Viewport size
};
```

### World to Screen Transformation

```cpp
// Get camera
auto &camera = rendering_engine->GetCamera();

// Set camera position (follow player)
camera.position = player_pos;

// Convert world coordinates to screen coordinates
Vector2f world_pos{1000.0f, 500.0f};
Vector2f screen_pos = camera.WorldToScreen(world_pos);

// Example: Render sprite at world position
Engine::Video::Transform transform;
transform.position = camera.WorldToScreen(entity.world_position);
rendering_engine->RenderSprite("entity", transform, nullptr, Color::White, 1);
```

### Frustum Culling

```cpp
// Check if entity is visible before rendering
if (camera.IsVisible(entity_pos, entity_size)) {
    RenderEntity(entity);
} else {
    // Skip rendering - entity is off-screen
}
```

:::tip Performance
Always use frustum culling for large numbers of entities to avoid rendering off-screen objects.
:::

## Render Statistics

### Getting Statistics

```cpp
const auto &stats = rendering_engine->GetStats();

std::cout << "Sprite draws: " << stats.sprite_draw_calls << std::endl;
std::cout << "Text draws: " << stats.text_draw_calls << std::endl;
std::cout << "Particle batches: " << stats.particle_batches << std::endl;
std::cout << "Total particles: " << stats.total_particles << std::endl;
```

### Resetting Statistics

```cpp
// Reset stats at start of frame
rendering_engine->ResetStats();

// Render everything...

// Check stats at end of frame
auto stats = rendering_engine->GetStats();
if (stats.sprite_draw_calls > 1000) {
    std::cerr << "Warning: Too many draw calls!" << std::endl;
}
```

## Complete Example

Here's a complete example showing typical usage in a game system:

```cpp
#include <rendering/RenderingEngine.hpp>

class GameRenderer {
private:
    Engine::Rendering::RenderingEngine *rendering_engine_;
    
public:
    GameRenderer(Engine::Rendering::RenderingEngine *engine)
        : rendering_engine_(engine) {
        // Load resources
        rendering_engine_->LoadTexture("player", "assets/player.png");
        rendering_engine_->LoadTexture("enemy", "assets/enemy.png");
        rendering_engine_->LoadFont("main_font", "assets/dogica.ttf");
        rendering_engine_->LoadShader("wave", "shaders/wave.vert", "shaders/wave.frag");
    }
    
    ~GameRenderer() {
        // Cleanup resources
        rendering_engine_->UnloadTexture("player");
        rendering_engine_->UnloadTexture("enemy");
        rendering_engine_->UnloadFont("main_font");
        rendering_engine_->UnloadShader("wave");
    }
    
    void RenderFrame(const GameState &state) {
        // Begin frame
        rendering_engine_->BeginFrame(Engine::Graphics::Color(20, 20, 40, 255));
        
        // Reset statistics
        rendering_engine_->ResetStats();
        
        // Update camera to follow player
        auto &camera = rendering_engine_->GetCamera();
        camera.position = state.player.position;
        
        // Render background layer (z=0)
        RenderBackground();
        
        // Render entities (z=1)
        RenderPlayer(state.player);
        for (const auto &enemy : state.enemies) {
            RenderEnemy(enemy);
        }
        
        // Render particle effects (z=2)
        RenderParticleEffects(state.particles);
        
        // Render UI (z=10 - always on top)
        RenderUI(state.score, state.health);
        
        // End frame
        rendering_engine_->EndFrame();
        
        // Log performance stats
        const auto &stats = rendering_engine_->GetStats();
        if (stats.sprite_draw_calls > 500) {
            DEBUG_RENDERING_LOG("High draw call count: " << stats.sprite_draw_calls);
        }
    }
    
private:
    void RenderPlayer(const Player &player) {
        if (!rendering_engine_->GetCamera().IsVisible(player.position, {32, 32})) {
            return;  // Off-screen, skip
        }
        
        Engine::Video::Transform transform;
        transform.position = rendering_engine_->GetCamera().WorldToScreen(player.position);
        transform.rotation = player.rotation;
        transform.scale = {1.0f, 1.0f};
        transform.origin = {16.0f, 16.0f};  // Center of 32x32 sprite
        
        // Apply shader effect
        rendering_engine_->SetShaderParameter("wave", "time", player.hit_flash_time);
        
        Engine::Graphics::IntRect frame(
            player.animation_frame * 32, 0, 32, 32
        );
        
        rendering_engine_->RenderSprite(
            "player",
            transform,
            &frame,
            player.is_hit ? Engine::Graphics::Color(255, 100, 100, 255) : Engine::Graphics::Color::White,
            1  // z_index
        );
    }
    
    void RenderUI(int score, int health) {
        Engine::Video::Transform transform;
        transform.position = {10.0f, 10.0f};
        
        rendering_engine_->RenderText(
            "Score: " + std::to_string(score),
            "main_font",
            transform,
            24,
            Engine::Graphics::Color::White,
            10  // High z_index for UI
        );
        
        // Health bar
        Engine::Graphics::FloatRect health_rect(10, 50, health * 2.0f, 20);
        rendering_engine_->DrawRectangle(
            health_rect,
            Engine::Graphics::Color(0, 255, 0, 255),
            &Engine::Graphics::Color::White,
            2.0f
        );
    }
};
```

## Best Practices

### ✅ DO:
- Always call `BeginFrame()` and `EndFrame()` every frame
- Validate `IsInitialized()` after initialization
- Use frustum culling for off-screen entities
- Load textures once, reuse texture IDs
- Use reference counting (multiple LoadTexture calls are safe)
- Reset statistics at the start of each frame
- Use appropriate z_index values for layering

### ❌ DON'T:
- Don't render between frames (without BeginFrame/EndFrame)
- Don't load the same texture with different IDs
- Don't forget to unload resources in cleanup
- Don't render every entity without culling
- Don't ignore null plugin checks in custom code
- Don't mix world and screen coordinates without transformation

## Error Handling

The RenderingEngine uses **exceptions for critical failures** and **return values for resource operations**:

### Critical Operations (Throw Exceptions):
```cpp
try {
    rendering_engine->Initialize(1920, 1080, "Game");
    rendering_engine->BeginFrame(Color::Black);
    rendering_engine->EndFrame();
} catch (const std::runtime_error &e) {
    std::cerr << "Critical error: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
```

### Resource Operations (Return bool):
```cpp
if (!rendering_engine->LoadTexture("player", "assets/player.png")) {
    // Handle gracefully - maybe use placeholder texture
    std::cerr << "Warning: Failed to load player texture" << std::endl;
}
```

## See Also

- [Plugin System Architecture](../plugins/architecture.md)
- [IVideoModule API Reference](../plugins/api-reference.md)
- [Debug Build System](./debug-system.md)
- [Creating Custom Video Plugins](../plugins/video-plugin-guide.md)
