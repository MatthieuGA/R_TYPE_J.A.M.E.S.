# SFML Backend Implementation

## Overview

`SFMLRenderContext` is the concrete implementation of `IRenderContext` using SFML (Simple Fast Multimedia Library) version 2.6.1+. It manages all SFML-specific graphics state, resource caching, and rendering operations.

The `SFMLRenderContext` is the concrete graphics backend implementation.

**Key Responsibility:** Abstract SFML details away from systems while providing efficient resource management and rendering.

## Architecture

### Class Structure

```cpp
class SFMLRenderContext : public IRenderContext {
 private:
  sf::RenderWindow& window_;           // Reference to SFML window

  // Resource caches
  std::unordered_map<std::string, sf::Texture> texture_cache_;
  std::unordered_map<std::string, sf::Font> font_cache_;
  std::unordered_map<std::string, sf::Shader> shader_cache_;

  // View management
  sf::View game_view_;                 // Camera/viewport
  bool view_dirty_;                    // Needs update

 public:
  explicit SFMLRenderContext(sf::RenderWindow& window);

  // === IRenderContext implementation ===
  void DrawSprite(const DrawSpriteParams& params) override;
  void DrawText(const DrawTextParams& params) override;
  void DrawRectangle(const DrawRectangleParams& params) override;
  void DrawVertexArray(const VertexArrayParams& params) override;
  void DrawableShader(const DrawableShaderParams& params) override;

  sf::Vector2f GetTextureSize(const std::string& texture_key) override;
  sf::FloatRect GetTextBounds(const std::string& font_key,
                               const std::string& text,
                               unsigned int char_size) override;
  sf::Vector2i GetGridFrameSize(const std::string& texture_key,
                                 int grid_cols,
                                 int frame_width) override;

  void Display() override;
  void Clear(const sf::Color& color) override;

  // === Additional backend-specific methods ===
  void LoadTexture(const std::string& key, const std::string& path);
  void LoadFont(const std::string& key, const std::string& path);
  void LoadShader(const std::string& key, const std::string& vert_path,
                   const std::string& frag_path);

  void SetView(const sf::View& view);
  void UpdateView(const sf::Vector2f& center, const sf::Vector2f& size);
};
```

### Resource Caching Strategy

All resources are loaded once and reused:

| Resource Type | Cache | Lifetime | Thread-Safe |
|---|---|---|---|
| **Texture** | `texture_cache_` | Application | No (loaded in main thread) |
| **Font** | `font_cache_` | Application | No (loaded in main thread) |
| **Shader** | `shader_cache_` | Application | No (loaded in main thread) |

**Why caching matters:**
- Loading a texture from disk is slow (~5–50 ms depending on size).
- Fonts require parsing and glyph rasterization.
- Reusing cached resources means consistent frame times.

## Resource Loading

### Loading Textures

**Method:**
```cpp
void SFMLRenderContext::LoadTexture(const std::string& key,
                                     const std::string& path)
```

**Behavior:**
1. Check if `key` already exists in `texture_cache_`. If yes, return early (no reload).
2. Load texture from disk using SFML's `sf::Texture::LoadFromFile(path)`.
3. If loading fails, log error and return without caching.
4. Cache the texture with the given `key`.
5. Store texture dimensions for later queries via `GetTextureSize`.

**Example (from client initialization):**
```cpp
render_context.LoadTexture("player", "assets/player.png");
render_context.LoadTexture("enemy_wave_1", "assets/enemies/wave1.png");
render_context.LoadTexture("particles", "assets/particles.png");
```

### Loading Fonts

**Method:**
```cpp
void SFMLRenderContext::LoadFont(const std::string& key,
                                  const std::string& path)
```

**Behavior:** Similar to textures; caches font objects for text rendering.

**Example:**
```cpp
render_context.LoadFont("default", "assets/fonts/arial.ttf");
render_context.LoadFont("ui", "assets/fonts/ui_font.ttf");
```

### Loading Shaders

**Method:**
```cpp
void SFMLRenderContext::LoadShader(const std::string& key,
                                    const std::string& vert_path,
                                    const std::string& frag_path)
```

**Behavior:**
1. Check if `key` already in `shader_cache_`. If yes, return early.
2. Load vertex and fragment shaders from files using SFML's `sf::Shader::LoadFromFile`.
3. If either fails, log error and return.
4. Cache the compiled shader with the given `key`.

**Limitations (SFML 2.6.1):**
- SFML does not support geometry or compute shaders.
- Only vertex + fragment shaders are supported.
- Shader uniforms are set via `setUniform()` on the `sf::Shader` object.

**Example:**
```cpp
render_context.LoadShader("charged_glow",
  "assets/shaders/charged.vert", "assets/shaders/charged.frag");
```

## Drawing Operations

### DrawSprite Implementation

```cpp
void SFMLRenderContext::DrawSprite(const DrawSpriteParams& params) {
  // 1. Look up texture
  auto it = texture_cache_.find(params.texture_key);
  if (it == texture_cache_.end()) {
    LogWarning("Texture not found: " + params.texture_key);
    return;
  }

  // 2. Create SFML sprite
  sf::Sprite sprite(it->second);

  // 3. Set position and origin
  sprite.setPosition(params.position);
  sprite.setOrigin(params.origin);

  // 4. Set scale, rotation, and color
  sprite.setScale(params.scale);
  sprite.setRotation(params.rotation);
  sprite.setColor(params.color);

  // 5. Set texture region (for atlases and animations)
  if (params.source_rect.width != -1) {
    sprite.setTextureRect(params.source_rect);
  }

  // 6. Render to window
  window_.draw(sprite);
}
```

**Key Points:**
- The `origin` parameter directly maps to SFML's `setOrigin()`, which sets the pivot point for rotation and scaling.
- The `position` parameter is the world position; SFML's view automatically transforms it.
- The `source_rect` allows rendering sub-regions, enabling animation strips and atlases.

### DrawText Implementation

```cpp
void SFMLRenderContext::DrawText(const DrawTextParams& params) {
  // 1. Look up font
  auto it = font_cache_.find(params.font_key);
  if (it == font_cache_.end()) {
    LogWarning("Font not found: " + params.font_key);
    return;
  }

  // 2. Create SFML text
  sf::Text text(params.text, it->second, params.character_size);

  // 3. Set position and origin
  text.setPosition(params.position);
  text.setOrigin(params.origin);

  // 4. Set color, rotation, and scale
  text.setFillColor(params.color);
  text.setRotation(params.rotation);
  text.setScale(params.scale);

  // 5. Render
  window_.draw(text);
}
```

**Important:** The `origin` is typically calculated by the caller using `GetTextBounds()` to center text on the position.

### DrawRectangle Implementation

```cpp
void SFMLRenderContext::DrawRectangle(const DrawRectangleParams& params) {
  sf::RectangleShape rect(params.size);
  rect.setPosition(params.position);
  rect.setOrigin(params.origin);
  rect.setScale(params.scale);
  rect.setRotation(params.rotation);
  rect.setFillColor(params.fill_color);

  if (params.outline_thickness > 0) {
    rect.setOutlineColor(params.outline_color);
    rect.setOutlineThickness(params.outline_thickness);
  }

  window_.draw(rect);
}
```

### DrawVertexArray Implementation

```cpp
void SFMLRenderContext::DrawVertexArray(const VertexArrayParams& params) {
  // 1. Set up render state
  sf::RenderStates states;

  // 2. Bind texture if provided
  if (!params.texture_key.empty()) {
    auto it = texture_cache_.find(params.texture_key);
    if (it != texture_cache_.end()) {
      states.texture = &it->second;
    }
  }

  // 3. Bind shader if provided
  if (!params.shader_key.empty()) {
    auto it = shader_cache_.find(params.shader_key);
    if (it != shader_cache_.end()) {
      states.shader = &it->second;
    }
  }

  // 4. Render vertices or indexed geometry
  if (params.indices.empty()) {
    // Direct rendering of vertex array
    sf::VertexArray va(sf::PrimitiveType::Triangles, params.vertices.size());
    for (size_t i = 0; i < params.vertices.size(); ++i) {
      va[i] = params.vertices[i];
    }
    window_.draw(va, states);
  } else {
    // Indexed rendering (for efficiency)
    // Note: SFML doesn't natively support index buffers;
    // this is simulated by reordering vertices
    sf::VertexArray va(sf::PrimitiveType::Triangles, params.indices.size());
    for (size_t i = 0; i < params.indices.size(); ++i) {
      va[i] = params.vertices[params.indices[i]];
    }
    window_.draw(va, states);
  }
}
```

### DrawableShader Implementation

```cpp
void SFMLRenderContext::DrawableShader(const DrawableShaderParams& params) {
  auto shader_it = shader_cache_.find(params.shader_key);
  if (shader_it == shader_cache_.end()) {
    LogWarning("Shader not found: " + params.shader_key);
    return;
  }

  auto texture_it = texture_cache_.find(params.texture_key);
  if (texture_it == texture_cache_.end()) {
    LogWarning("Texture not found: " + params.texture_key);
    return;
  }

  // Set built-in uniforms
  shader_it->second.setUniform("u_time", elapsed_time_);
  shader_it->second.setUniform("u_texture", texture_it->second);
  shader_it->second.setUniform("u_position", params.position);
  shader_it->second.setUniform("u_scale", params.scale);
  shader_it->second.setUniform("u_frame", static_cast<float>(params.frame));

  // Note: Actual shader application is done by passing the shader
  // to sf::RenderStates and calling window_.draw()
  // This method primarily sets up the uniforms.
}
```

## Resource Queries

### GetTextureSize

```cpp
sf::Vector2f SFMLRenderContext::GetTextureSize(
    const std::string& texture_key) {
  auto it = texture_cache_.find(texture_key);
  if (it == texture_cache_.end()) {
    LogWarning("Texture not found: " + texture_key);
    return {0, 0};
  }

  auto size = it->second.getSize();
  return {static_cast<float>(size.x), static_cast<float>(size.y)};
}
```

**Use Case:** Systems query this to determine sprite bounds and set up animation frames.

### GetTextBounds

```cpp
sf::FloatRect SFMLRenderContext::GetTextBounds(
    const std::string& font_key,
    const std::string& text,
    unsigned int char_size) {
  auto it = font_cache_.find(font_key);
  if (it == font_cache_.end()) {
    LogWarning("Font not found: " + font_key);
    return {0, 0, 0, 0};
  }

  sf::Text temp(text, it->second, char_size);
  return temp.getLocalBounds();
}
```

**Use Case:** Systems query this to center text or calculate UI dimensions.

### GetGridFrameSize

```cpp
sf::Vector2i SFMLRenderContext::GetGridFrameSize(
    const std::string& texture_key,
    int grid_cols,
    int frame_width) {
  auto texture_size = GetTextureSize(texture_key);
  int frame_height =
    static_cast<int>(texture_size.y) * frame_width /
    (grid_cols * static_cast<int>(texture_size.x));

  return {frame_width, frame_height};
}
```

**Use Case:** Animation systems calculate frame dimensions for grid-based atlases.

## View and Camera Management

### SetView

```cpp
void SFMLRenderContext::SetView(const sf::View& view) {
  game_view_ = view;
  window_.setView(game_view_);
  view_dirty_ = false;
}
```

**Use Case:** Switching camera views (main game, UI layer, etc.).

### UpdateView

```cpp
void SFMLRenderContext::UpdateView(const sf::Vector2f& center,
                                    const sf::Vector2f& size) {
  game_view_.setCenter(center);
  game_view_.setSize(size);
  window_.setView(game_view_);
  view_dirty_ = false;
}
```

**Use Case:** Smooth camera follow, zoom, or viewport changes.

## Display and Lifecycle

### Clear

```cpp
void SFMLRenderContext::Clear(const sf::Color& color) {
  window_.clear(color);
}
```

**Called:** Once per frame before any draws.

### Display

```cpp
void SFMLRenderContext::Display() {
  window_.display();
}
```

**Called:** Once per frame after all draws. Presents the back buffer to the screen.

## Performance Considerations

### Texture Atlasing

Instead of many small texture files, combine multiple sprites onto a single large texture. Specify the `source_rect` in `DrawSpriteParams` to render the correct region.

**Benefits:**
- Reduced state changes (one texture bind per draw call vs. many)
- Better GPU cache locality
- Faster loading (fewer file I/O operations)

**Example:**
```cpp
// Player on a 256×512 sheet at row 0
DrawSpriteParams params{
  .texture_key = "player",
  .source_rect = {0, 0, 64, 128}  // First frame
};
// Animation advances source_rect
```

### Batch Rendering

The vertex array (`DrawVertexArray`) is efficient for many small objects (particles, bullet trails). A single draw call with 1000 vertices is faster than 1000 separate sprite draws.

### Shader Overhead

Shaders add overhead. Use judiciously:
- Sparingly for special effects (charged projectile glow)
- Avoid per-entity shaders in tight loops
- Prefer pre-rendered assets for static effects

## Error Handling and Logging

Missing resources are handled gracefully:
- Missing texture: Log warning, continue rendering (entity not visible)
- Missing font: Log warning, text not rendered
- Missing shader: Log warning, use default rendering

**Recommendation:** Validate resource keys at entity creation time; log missing resources to the console/file for debugging.

## Thread Safety

**Not thread-safe.** All graphics operations must happen on the main thread (where SFML's window was created). If using multi-threaded systems:
1. Collect render commands from worker threads into a thread-safe queue.
2. Process the queue on the main thread before calling render systems.
3. Or use lock-based synchronization (performance penalty).

## Testing with a Mock Backend

For unit testing without SFML:
1. Create a `MockRenderContext` implementing `IRenderContext`.
2. Return dummy values (e.g., `{64, 64}` for texture size).
3. Track draw call counts and parameters for assertions.
4. Pass the mock to systems during testing.

**Example:**
```cpp
class MockRenderContext : public IRenderContext {
 public:
  int draw_sprite_calls = 0;

  void DrawSprite(const DrawSpriteParams& params) override {
    draw_sprite_calls++;
  }

  // ... other methods return dummy values
};

TEST(DrawableSystemTest, RendersCachedSprites) {
  MockRenderContext mock;
  auto registry = InitializeMockRegistry(mock);

  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {"texture", {0, 0}, 0, 0, {1, 1}, true, ""});

  DrawableSystem::Update(registry, mock, 0.016f);

  EXPECT_EQ(mock.draw_sprite_calls, 1);
}
```

## References

- [IRenderContext Reference](./irendercontext.md)
- [SFML Documentation](https://www.sfml-dev.org/)
- [Architecture Overview](./overview.md)
