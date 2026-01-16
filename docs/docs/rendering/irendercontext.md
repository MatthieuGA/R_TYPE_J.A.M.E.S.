# IRenderContext API Reference

## Overview

`IRenderContext` is the abstract interface through which all rendering operations and resource queries are performed. It defines the contract between the ECS systems and the graphics backend, ensuring complete decoupling from SFML or any specific graphics library.

**Location:** `engine/include/graphics/IRenderContext.hpp`

**Inheritance:** All backends (e.g., `SFMLRenderContext`) inherit from this pure abstract class.

## Interface Definition

```cpp
class IRenderContext {
 public:
  virtual ~IRenderContext() = default;

  // === Drawing Operations ===
  virtual void DrawSprite(const DrawSpriteParams& params) = 0;
  virtual void DrawText(const DrawTextParams& params) = 0;
  virtual void DrawRectangle(const DrawRectangleParams& params) = 0;
  virtual void DrawVertexArray(const VertexArrayParams& params) = 0;
  
  // === Shader Operations ===
  virtual void DrawableShader(const DrawableShaderParams& params) = 0;
  
  // === Resource Queries ===
  virtual sf::Vector2f GetTextureSize(const std::string& texture_key) = 0;
  virtual sf::FloatRect GetTextBounds(const std::string& font_key,
                                       const std::string& text,
                                       unsigned int char_size) = 0;
  virtual sf::Vector2i GetGridFrameSize(const std::string& texture_key,
                                         int grid_cols,
                                         int frame_width) = 0;

  // === Display ===
  virtual void Display() = 0;
  virtual void Clear(const sf::Color& color) = 0;
};
```

## Draw Operations

### DrawSprite

Renders a textured rectangle to the screen.

**Parameters:**
```cpp
struct DrawSpriteParams {
  std::string texture_key;      // Resource ID (e.g., "player")
  sf::Vector2f position;        // World position (top-left)
  sf::Vector2f origin;          // Local offset from position
  sf::Vector2f scale;           // X, Y scaling
  float rotation;               // Degrees (CCW)
  sf::IntRect source_rect;      // Texture region (for atlases/frames)
  sf::Color color;              // Tint (default: white, fully opaque)
};
```

**Contract:**
- `texture_key` must exist in the backend's cache. If not, backend may throw or return silently (see backend docs).
- `position` is in game world coordinates; backend handles camera/view transformation.
- `origin` is the offset from position that becomes the pivot for rotation and scaling.
- `source_rect` allows drawing a sub-region of the texture; use `{0, 0, -1, -1}` for full texture.

**Example (from DrawableSystem):**
```cpp
DrawSpriteParams params{
  .texture_key = drawable.texture_key_,
  .position = transform.position_,
  .origin = drawable.origin_,
  .scale = drawable.scale_,
  .rotation = drawable.rotation_,
  .source_rect = frame_rect,  // Determined by AnimationFrame
  .color = sf::Color::White
};
render_context->DrawSprite(params);
```

### DrawText

Renders text to the screen.

**Parameters:**
```cpp
struct DrawTextParams {
  std::string font_key;         // Resource ID (e.g., "default")
  std::string text;             // String to render
  sf::Vector2f position;        // World position
  sf::Vector2f origin;          // Local offset (usually text bounds-based)
  unsigned int character_size;  // Font size in pixels
  sf::Color color;              // Text color
  float rotation;               // Degrees (CCW)
  sf::Vector2f scale;           // X, Y scaling
};
```

**Contract:**
- `font_key` must exist in the backend's cache.
- `position` is the anchor point; `origin` offsets from it.
- `character_size` is the font size in pixels.
- To center text, caller computes `origin` as half of `GetTextBounds(...).size`.

**Example (from DrawTextSystem):**
```cpp
auto bounds = render_context->GetTextBounds(
  draw_text.font_key_, draw_text.text_, draw_text.character_size_);
sf::Vector2f center_offset = bounds.size / 2.0f;

DrawTextParams params{
  .font_key = draw_text.font_key_,
  .text = draw_text.text_,
  .position = transform.position_,
  .origin = center_offset,
  .character_size = draw_text.character_size_,
  .color = draw_text.color_,
  .rotation = draw_text.rotation_,
  .scale = draw_text.scale_
};
render_context->DrawText(params);
```

### DrawRectangle

Renders a filled or outlined rectangle.

**Parameters:**
```cpp
struct DrawRectangleParams {
  sf::Vector2f position;        // Top-left corner
  sf::Vector2f size;            // Width and height
  sf::Vector2f origin;          // Local offset
  sf::Vector2f scale;           // X, Y scaling
  float rotation;               // Degrees (CCW)
  sf::Color fill_color;         // Interior color (or transparent)
  sf::Color outline_color;      // Border color (or transparent)
  float outline_thickness;      // Outline width in pixels
};
```

**Contract:**
- If `fill_color` has alpha 0 and `outline_thickness` > 0, only the border is drawn.
- If `outline_thickness` is 0, no border is drawn.
- Origin and scale work the same as sprites: offset and pivot for rotation.

### DrawVertexArray

Renders a custom geometry defined by vertices and indices.

**Parameters:**
```cpp
struct VertexArrayParams {
  std::vector<sf::Vertex> vertices;     // Position, color, texture coords
  std::vector<unsigned int> indices;    // Triangulation (optional)
  std::string texture_key;              // Resource ID (or empty for untextured)
  std::string shader_key;               // Shader to apply (or empty)
};
```

**Contract:**
- Vertices should be in world space; backend handles view transformation.
- If `indices` is empty, vertices are rendered as-is (e.g., points, lines, triangle strip).
- If `indices` is non-empty, vertices are indexed in triangles (every 3 indices = 1 triangle).
- If `texture_key` is empty, rendering is untextured (uses vertex colors).
- If `shader_key` is non-empty, the shader is applied during rendering.

**Typical Use Case (Particles):**
```cpp
VertexArrayParams params{
  .vertices = particle_quads,  // 4 vertices per quad
  .indices = indices,           // Tri-strip pattern: 0,1,2, 1,2,3, 4,5,6, ...
  .texture_key = "particles",
  .shader_key = ""              // Use default shader
};
render_context->DrawVertexArray(params);
```

## Shader Operations

### DrawableShader

Applies a shader effect to a drawable entity.

**Parameters:**
```cpp
struct DrawableShaderParams {
  std::string shader_key;       // Resource ID (e.g., "charged_glow")
  std::string texture_key;      // Texture to sample
  sf::Vector2f position;        // Entity position
  sf::Vector2f scale;           // Entity scale
  int frame;                    // Current animation frame (for grid/strip)
  float elapsed_time;           // Animation progress (seconds)
};
```

**Contract:**
- `shader_key` must exist in the backend's shader cache.
- The shader receives built-in uniforms: `u_time`, `u_texture`, position and scale info.
- Custom uniforms can be defined per-shader in the backend configuration.

**Example (from charged projectile system):**
```cpp
if (!drawable.shader_key_.empty()) {
  DrawableShaderParams shader_params{
    .shader_key = drawable.shader_key_,
    .texture_key = drawable.texture_key_,
    .position = transform.position_,
    .scale = drawable.scale_,
    .frame = drawable.frame_,
    .elapsed_time = animation.elapsed_time_
  };
  render_context->DrawableShader(shader_params);
}
```

## Resource Queries

### GetTextureSize

Returns the dimensions of a loaded texture.

**Signature:**
```cpp
sf::Vector2f GetTextureSize(const std::string& texture_key);
```

**Returns:** `{width, height}` in pixels. Throws or logs error if texture not found.

**Use Case:** Determining sprite bounds and setting up animation frame calculations.

**Example:**
```cpp
auto size = render_context->GetTextureSize("player");
drawable.origin_ = size / 2.0f;  // Center sprite origin
```

### GetTextBounds

Returns the bounding rectangle of rendered text.

**Signature:**
```cpp
sf::FloatRect GetTextBounds(const std::string& font_key,
                             const std::string& text,
                             unsigned int char_size);
```

**Returns:** `{left, top, width, height}` — the bounds of the rendered text in local coordinates (not world space).

**Use Case:** Centering text, calculating UI layout dimensions.

**Example:**
```cpp
auto bounds = render_context->GetTextBounds("default", "Score: 1000", 32);
// bounds.size = {width, height} of the text
sf::Vector2f center = bounds.size / 2.0f;
draw_text.origin_ = center;  // Center text on its position
```

### GetGridFrameSize

Returns the size of a single frame in a grid-based texture atlas.

**Signature:**
```cpp
sf::Vector2i GetGridFrameSize(const std::string& texture_key,
                               int grid_cols,
                               int frame_width);
```

**Parameters:**
- `texture_key`: Resource ID
- `grid_cols`: Number of columns in the grid
- `frame_width`: Width of one frame in pixels

**Returns:** `{frame_width, frame_height}` — calculated from grid layout.

**Use Case:** Animation systems that use grid-based atlases (e.g., 4×8 sprites on a single texture).

## Display and Lifecycle

### Clear

Clears the screen to a background color.

**Signature:**
```cpp
void Clear(const sf::Color& color);
```

**Contract:** Called once per frame before any draws. Typically `sf::Color::Black` or `sf::Color(20, 20, 30)`.

### Display

Presents the rendered frame to the screen.

**Signature:**
```cpp
void Display();
```

**Contract:** Called once per frame after all draws. Flips the back buffer to the front buffer (double buffering).

## Error Handling

The interface does not mandate specific error behavior. Backends may:
- Throw exceptions for missing resources
- Log warnings and continue
- Return default/zero values

**Recommendation:** The SFML backend logs warnings for missing resources and continues gracefully. Systems should not assume exceptions and should validate resource keys before rendering.

## Extension Points

### Adding New Draw Operations

To support a new primitive (e.g., `DrawCircle`):
1. Add a new `Draw*Params` struct and `Draw*` method to `IRenderContext`.
2. Implement in each backend (e.g., `SFMLRenderContext`).
3. Update systems to use the new operation.

**Backwards Compatibility:** Existing systems continue to compile and work unchanged.

### Custom Shader Uniforms

Backends can extend shader behavior by:
1. Defining custom uniform sets in shader asset files
2. Populating them in `DrawableShader` based on entity data
3. No changes required to the interface; all happens within the backend

## References

- [IRenderContext Header](../../engine/include/graphics/IRenderContext.hpp)
- [SFML Backend Implementation](./sfml-backend.md)
- [Rendering Systems](./systems.md)
- [Architecture Overview](./overview.md)
