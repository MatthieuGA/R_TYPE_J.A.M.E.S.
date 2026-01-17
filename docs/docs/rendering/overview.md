# Rendering Overview

## Purpose and Scope

The R-Type J.A.M.E.S. rendering system is a **decoupled, backend-agnostic architecture** that abstracts graphics operations from the Entity-Component-System (ECS) engine. This design allows multiple graphics backends to coexist without modifying core engine logic, enabling flexibility, testability, and maintainability.

**Scope of this documentation:**
- High-level architecture and design philosophy
- Interface contracts and abstractions (`IRenderContext`)
- SFML-specific backend implementation
- System behavior (Drawable, DrawText, Animation)
- Initialization and runtime lifecycle
- Integration with the ECS registry

## Design Philosophy

### Principle 1: Decouple Graphics from Logic

All game logic, entity management, and system behavior **must not depend directly on SFML** or any graphics library. Instead, systems interact through a well-defined interface (`IRenderContext`), which is provided as a dependency to render systems.

**Benefit:** The engine remains graphics-agnostic, enabling:
- Backend swapping (SFML → Vulkan, OpenGL, etc.)
- Headless testing (mock renderer)
- Faster iteration on graphics features

### Principle 2: Backend Owns Resource Management

The backend (e.g., `SFMLRenderContext`) is responsible for:
- Loading and caching textures, fonts, and shaders
- Managing texture atlases and sprite sheets
- Querying resource properties (size, bounds, frame data)
- Rendering to the screen

Systems **never directly access** backend resources; they always go through the `IRenderContext` interface.

### Principle 3: Clear Separation of Concerns

| Layer | Responsibility |
|-------|---|
| **ECS Systems** | Decide *what* to draw and *where* (position, rotation, scale). Query resource properties via interface. |
| **IRenderContext** | Abstract drawing operations and resource queries. Define contracts. |
| **Backend (SFML)** | Implement rendering, manage resources, handle low-level graphics state. |

## Key Abstractions

### IRenderContext

An interface that systems use to:
1. **Draw primitives:** `DrawSprite`, `DrawText`, `DrawRectangle`, `DrawVertexArray`
2. **Apply effects:** Shaders via `DrawableShader`
3. **Query resources:** Texture size, text bounds, animation frame data
4. **Manage vertex arrays:** For complex shapes and particle effects

See [IRenderContext Reference](./irendercontext.md) for detailed API.

### Component Structure

**Drawable Component**
```cpp
struct Drawable {
  std::string texture_key_;     // Resource identifier (e.g., "player")
  sf::Vector2f origin_;          // Sprite center offset
  int frame_;                    // Current animation frame (0 for static)
  float rotation_;               // Degrees
  sf::Vector2f scale_;           // X and Y multipliers
  bool visible_;                 // Render or skip
  std::string shader_key_;       // Optional shader (e.g., "charged_glow")
};
```

**DrawText Component**
```cpp
struct DrawText {
  std::string text_;
  std::string font_key_;         // Resource identifier
  unsigned int character_size_;
  sf::Color color_;
  sf::Vector2f origin_;          // Text baseline offset
  float rotation_;
  sf::Vector2f scale_;
};
```

**AnimationFrame Component** (optional, for animated entities)
```cpp
struct AnimationFrame {
  int current_frame_;            // 0-indexed
  float elapsed_time_;           // Time in current frame
  float frame_duration_;         // Seconds per frame
  std::string animation_mode_;   // "strip" or "grid"
  int frame_count_;              // Total frames
  int frame_width_;              // For strip/grid
  int grid_cols_;                // For grid mode
};
```

## System Flow (High Level)

1. **Initialization** (see [Lifecycle](./lifecycle.md))
   - `InitializeDrawableStaticSystem`: Set up origin offsets for non-animated sprites
   - `InitializeAnimationSystem`: Configure animation metadata
   - `SFMLRenderContext`: Load textures, fonts, and shaders into cache

2. **Runtime Each Frame**
   - `DrawableSystem`: Iterate over drawable entities; query texture size; call `DrawSprite` with calculated origin
   - `AnimationSystem`: Update frame index; pass current frame to drawable
   - `DrawTextSystem`: Render text with computed bounds-based origin
   - `ParticleEmitterSystem`: Generate vertex arrays; render via `DrawVertexArray`

3. **Rendering** (Backend)
   - SFML window renders queued draw calls
   - Shaders applied per-draw for effects (e.g., charged projectile glow)

## Extensibility

### Adding a New Backend

1. Create a new class implementing `IRenderContext`:
   ```cpp
   class VulkanRenderContext : public IRenderContext {
     void DrawSprite(...) override { /* Vulkan code */ }
     // ... other methods
   };
   ```

2. Update [InitRegistrySystems](../architecture.md#initregistrysystems) to instantiate the new backend.

3. All systems continue to work unchanged.

### Custom Rendering

For special effects (e.g., screen-space distortion, post-processing):
- Extend `IRenderContext` with new methods (e.g., `ApplyPostProcess`)
- Implement in the backend
- Call from a custom system

## Benefits Achieved

| Goal | How Achieved |
|------|---|
| **No SFML leakage** | All includes isolated to backend; systems use `IRenderContext` |
| **Testability** | Mock backend for unit tests; no window/graphics setup required |
| **Flexibility** | Swap backends by changing one line; add new effects without touching systems |
| **Performance** | Backend caches resources; systems pass references, not copies |
| **Maintainability** | Clear contracts; centralized graphics logic in one place |

## References

- [IRenderContext API](./irendercontext.md)
- [SFML Backend Implementation](./sfml-backend.md)
- [Rendering Systems](./systems.md)
- [Initialization Lifecycle](./lifecycle.md)
- [Main Architecture](../architecture.md)
