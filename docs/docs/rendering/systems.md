# Rendering Systems

## Overview

Rendering systems are responsible for translating entity component data into draw commands submitted to the graphics backend. Each system specializes in a specific type of visual element.

**Key Principles:**
1. Systems do not directly use SFML; all draws go through `IRenderContext`.
2. Systems are stateless; they operate on entity components each frame.
3. Initialization systems (run once) set up component properties; update systems (run every frame) perform rendering.

## System Registry

All rendering systems are registered in the ECS registry during engine initialization. See [Initialization Lifecycle](./lifecycle.md) for the startup sequence.

### Registered Systems

| System | Phase | Frequency | Purpose |
|--------|-------|-----------|---------|
| `InitializeDrawableStaticSystem` | Init | Once | Set origin offsets for non-animated sprites |
| `InitializeAnimationSystem` | Init | Once | Configure animation metadata |
| `AnimationSystem` | Update | Every frame | Update animation frame indices |
| `DrawableSystem` | Update | Every frame | Render drawable entities (sprites) |
| `DrawTextSystem` | Update | Every frame | Render text entities |
| `ParticleEmitterSystem` | Update | Every frame | Generate and render particles |

## InitializeDrawableStaticSystem

### Purpose

Sets the `origin_` field of `Drawable` components for non-animated sprites. The origin is used as the pivot point for rotation and scaling, and aligning it to the sprite's center improves visual correctness.

### Signature

```cpp
void InitializeDrawableStaticSystem(
    const Registry& registry,
    IRenderContext& render_context);
```

### Behavior

1. Iterate over all entities with a `Drawable` component.
2. Check if the entity also has an `AnimationFrame` component.
3. **If no AnimationFrame (static sprite):**
   - Query texture size from `render_context.GetTextureSize(drawable.texture_key_)`.
   - Set `drawable.origin_ = texture_size / 2.0f` (center of the sprite).
4. **If AnimationFrame exists:** Skip (animation system handles origin initialization).

### Code Example

```cpp
void InitializeDrawableStaticSystem(const Registry& registry,
                                     IRenderContext& render_context) {
  auto drawable_view = registry.View<Drawable, Transform>();
  
  for (auto entity : drawable_view) {
    auto& drawable = registry.GetComponent<Drawable>(entity);
    
    // Skip animated entities; they handle origin in AnimationSystem
    if (registry.HasComponent<AnimationFrame>(entity)) {
      continue;
    }
    
    // Get texture size and center the origin
    auto texture_size = render_context.GetTextureSize(drawable.texture_key_);
    drawable.origin_ = texture_size / 2.0f;
  }
}
```

### When to Run

- Called once during engine initialization (before the game loop starts).
- Only processes static sprites; animated sprites are handled separately.

## InitializeAnimationSystem

### Purpose

Configures animation metadata for entities with animated sprites. This includes frame counts, durations, and animation mode (strip vs. grid).

### Signature

```cpp
void InitializeAnimationSystem(
    const Registry& registry,
    IRenderContext& render_context);
```

### Behavior

1. Iterate over entities with both `Drawable` and `AnimationFrame` components.
2. **For each entity:**
   - Validate animation mode (`"strip"` or `"grid"`).
   - Query texture size using `render_context.GetTextureSize(drawable.texture_key_)`.
   - **If mode is "strip":**
     - Calculate frame height: `texture_height / frame_count`.
     - Frames are laid out horizontally: frame 0 is `{0, 0, frame_width, frame_height}`, frame 1 is `{frame_width, 0, frame_width, frame_height}`, etc.
   - **If mode is "grid":**
     - Calculate frame height: `(texture_height / grid_rows)` where `grid_rows = ceil(frame_count / grid_cols)`.
     - Frames are laid out in a grid: frame index maps to row and column.
   - Set `drawable.origin_ = frame_size / 2.0f` (center each frame).

### Code Example

```cpp
void InitializeAnimationSystem(const Registry& registry,
                                IRenderContext& render_context) {
  auto anim_view = registry.View<Drawable, AnimationFrame>();
  
  for (auto entity : anim_view) {
    auto& drawable = registry.GetComponent<Drawable>(entity);
    auto& anim = registry.GetComponent<AnimationFrame>(entity);
    
    auto texture_size = render_context.GetTextureSize(drawable.texture_key_);
    
    if (anim.animation_mode_ == "strip") {
      // Horizontal strip: frames laid out left to right
      int frame_height = static_cast<int>(texture_size.y) / anim.frame_count_;
      drawable.origin_ = {anim.frame_width_ / 2.0f, frame_height / 2.0f};
    } else if (anim.animation_mode_ == "grid") {
      // Grid: frames in rows and columns
      int grid_rows = (anim.frame_count_ + anim.grid_cols_ - 1) / anim.grid_cols_;
      int frame_height = static_cast<int>(texture_size.y) / grid_rows;
      drawable.origin_ = {anim.frame_width_ / 2.0f, frame_height / 2.0f};
    }
  }
}
```

### Animation Modes

#### Strip Mode

Frames are arranged horizontally in a single row.

```
┌─────────┬─────────┬─────────┐
│ Frame 0 │ Frame 1 │ Frame 2 │
└─────────┴─────────┴─────────┘
Texture width = 3 * frame_width
Texture height = 1 * frame_height
```

**Usage:** Simple linear animations (walk cycle, attack animation).

#### Grid Mode

Frames are arranged in a grid (rows × columns).

```
┌─────────┬─────────┬─────────┬─────────┐
│ Frame 0 │ Frame 1 │ Frame 2 │ Frame 3 │
├─────────┼─────────┼─────────┼─────────┤
│ Frame 4 │ Frame 5 │ Frame 6 │ Frame 7 │
└─────────┴─────────┴─────────┴─────────┘
grid_cols = 4
grid_rows = 2
```

**Usage:** Complex assets with multiple directions or states (directional sprites, charged effects).

### When to Run

- Called once during engine initialization, after `InitializeDrawableStaticSystem`.
- Only processes entities with `AnimationFrame` components.

## AnimationSystem

### Purpose

Updates the current animation frame index each frame, advancing through the animation sequence and cycling when complete.

### Signature

```cpp
void AnimationSystem(
    const Registry& registry,
    float delta_time);
```

### Behavior

1. Iterate over entities with both `Drawable` and `AnimationFrame` components.
2. **For each entity:**
   - Increment `anim.elapsed_time_ += delta_time`.
   - If `elapsed_time_ >= frame_duration_`:
     - Advance to the next frame: `anim.current_frame_ = (anim.current_frame_ + 1) % anim.frame_count_`.
     - Reset elapsed time: `anim.elapsed_time_ = 0`.
   - Calculate the texture region (`source_rect`) based on current frame and animation mode.
   - Update `drawable.frame_` for use by `DrawableSystem`.

### Code Example

```cpp
void AnimationSystem(const Registry& registry, float delta_time) {
  auto anim_view = registry.View<Drawable, AnimationFrame>();
  
  for (auto entity : anim_view) {
    auto& drawable = registry.GetComponent<Drawable>(entity);
    auto& anim = registry.GetComponent<AnimationFrame>(entity);
    
    anim.elapsed_time_ += delta_time;
    
    if (anim.elapsed_time_ >= anim.frame_duration_) {
      anim.current_frame_ = (anim.current_frame_ + 1) % anim.frame_count_;
      anim.elapsed_time_ -= anim.frame_duration_;  // Carry over remainder
    }
    
    drawable.frame_ = anim.current_frame_;
  }
}
```

### Frame Calculation (Strip Mode)

```cpp
int frame_height = texture_height / frame_count;
source_rect = {
  current_frame * frame_width,  // Left
  0,                             // Top
  frame_width,                   // Width
  frame_height                   // Height
};
```

### Frame Calculation (Grid Mode)

```cpp
int grid_rows = (frame_count + grid_cols - 1) / grid_cols;
int frame_height = texture_height / grid_rows;
int row = current_frame / grid_cols;
int col = current_frame % grid_cols;
source_rect = {
  col * frame_width,             // Left
  row * frame_height,            // Top
  frame_width,                   // Width
  frame_height                   // Height
};
```

### Timing and Loop Behavior

- **Loop:** Animations loop by default (`current_frame % frame_count`).
- **Custom Timing:** Vary `frame_duration_` per frame by updating `AnimationFrame` during runtime.
- **One-Shot:** Implement by checking if `current_frame == frame_count - 1` and stopping advancement.

## DrawableSystem

### Purpose

Renders all drawable entities (sprites) to the screen. Uses texture regions determined by the animation system or static sprite data.

### Signature

```cpp
void DrawableSystem(
    const Registry& registry,
    IRenderContext& render_context,
    float delta_time);
```

### Behavior

1. Iterate over entities with both `Drawable` and `Transform` components.
2. **For each entity:**
   - Check if visible: `if (!drawable.visible_) continue;`.
   - Determine texture region:
     - **If AnimationFrame exists:** Use the current frame's `source_rect` (set by `AnimationSystem`).
     - **Otherwise:** Use full texture (`{0, 0, -1, -1}`).
   - Prepare draw parameters: position, origin, scale, rotation, color.
   - Call `render_context.DrawSprite(params)`.
   - **If shader requested:** Call `render_context.DrawableShader(shader_params)` for effects like charged projectile glow.

### Code Example

```cpp
void DrawableSystem(const Registry& registry,
                     IRenderContext& render_context,
                     float delta_time) {
  auto drawable_view = registry.View<Drawable, Transform>();
  
  for (auto entity : drawable_view) {
    auto& drawable = registry.GetComponent<Drawable>(entity);
    
    if (!drawable.visible_) continue;
    
    auto& transform = registry.GetComponent<Transform>(entity);
    
    // Determine source rectangle
    sf::IntRect source_rect{0, 0, -1, -1};  // Full texture by default
    if (registry.HasComponent<AnimationFrame>(entity)) {
      auto& anim = registry.GetComponent<AnimationFrame>(entity);
      // source_rect calculated by AnimationSystem, stored in drawable.frame_
      // (In practice, we'd compute it here again or cache it)
    }
    
    // Prepare and submit draw call
    DrawSpriteParams params{
      .texture_key = drawable.texture_key_,
      .position = transform.position_,
      .origin = drawable.origin_,
      .scale = drawable.scale_,
      .rotation = drawable.rotation_,
      .source_rect = source_rect,
      .color = sf::Color::White
    };
    
    render_context.DrawSprite(params);
    
    // Apply shader effects if requested
    if (!drawable.shader_key_.empty()) {
      DrawableShaderParams shader_params{
        .shader_key = drawable.shader_key_,
        .texture_key = drawable.texture_key_,
        .position = transform.position_,
        .scale = drawable.scale_,
        .frame = drawable.frame_,
        .elapsed_time = 0  // From animation component if available
      };
      render_context.DrawableShader(shader_params);
    }
  }
}
```

### Visibility Culling

The system skips invisible entities. Use `drawable.visible_ = false` to hide entities without removing them.

### Z-Ordering

SFML renders in the order draw calls are made. The iteration order over the registry determines rendering order. For explicit Z-ordering:
1. Add a `ZOrder` component to entities.
2. Sort the view by Z before rendering.
3. Or use SFML's layering system with multiple render targets.

## DrawTextSystem

### Purpose

Renders text entities to the screen. Handles font lookup, bounds calculation, and origin-based centering.

### Signature

```cpp
void DrawTextSystem(
    const Registry& registry,
    IRenderContext& render_context,
    float delta_time);
```

### Behavior

1. Iterate over entities with both `DrawText` and `Transform` components.
2. **For each entity:**
   - Query text bounds: `bounds = render_context.GetTextBounds(font_key, text, char_size)`.
   - Calculate center offset: `center = bounds.size / 2.0f`.
   - Update `draw_text.origin_ = center` (if not already set).
   - Prepare draw parameters: position, origin, font, text, color, size, rotation, scale.
   - Call `render_context.DrawText(params)`.

### Code Example

```cpp
void DrawTextSystem(const Registry& registry,
                    IRenderContext& render_context,
                    float delta_time) {
  auto text_view = registry.View<DrawText, Transform>();
  
  for (auto entity : text_view) {
    auto& draw_text = registry.GetComponent<DrawText>(entity);
    auto& transform = registry.GetComponent<Transform>(entity);
    
    // Query text bounds and compute center offset
    auto bounds = render_context.GetTextBounds(
      draw_text.font_key_, draw_text.text_, draw_text.character_size_);
    sf::Vector2f center = bounds.size / 2.0f;
    
    // Prepare draw parameters
    DrawTextParams params{
      .font_key = draw_text.font_key_,
      .text = draw_text.text_,
      .position = transform.position_,
      .origin = center,
      .character_size = draw_text.character_size_,
      .color = draw_text.color_,
      .rotation = draw_text.rotation_,
      .scale = draw_text.scale_
    };
    
    render_context.DrawText(params);
  }
}
```

### Dynamic Text Updates

Text can change per frame:
```cpp
draw_text.text_ = "Score: " + std::to_string(score);
// DrawTextSystem automatically re-calculates bounds next frame
```

### Font and Size Constraints

- Font size must be reasonable (e.g., 8–128 pixels).
- Very large fonts may degrade performance or quality.
- Pre-render common text to textures for static UI.

## ParticleEmitterSystem

### Purpose

Generates and renders particles for visual effects (explosions, bullet trails, environmental effects).

### Signature

```cpp
void ParticleEmitterSystem(
    const Registry& registry,
    IRenderContext& render_context,
    float delta_time);
```

### Behavior

1. Iterate over entities with `ParticleEmitter` and `Transform` components.
2. **For each emitter:**
   - Emit new particles based on emission rate and delta time.
   - Update all existing particles: position, velocity, lifetime, color, size.
   - Build a vertex array of particle quads.
   - Call `render_context.DrawVertexArray(params)` to render all particles at once.
   - Remove particles that have exceeded their lifetime.

### Code Example

```cpp
void ParticleEmitterSystem(const Registry& registry,
                           IRenderContext& render_context,
                           float delta_time) {
  auto emitter_view = registry.View<ParticleEmitter, Transform>();
  
  for (auto entity : emitter_view) {
    auto& emitter = registry.GetComponent<ParticleEmitter>(entity);
    auto& transform = registry.GetComponent<Transform>(entity);
    
    // Emit new particles
    int particles_to_emit = static_cast<int>(emitter.emission_rate_ * delta_time);
    for (int i = 0; i < particles_to_emit; ++i) {
      Particle p{
        .position = transform.position_,
        .velocity = RandomDirection() * emitter.initial_speed_,
        .lifetime = emitter.max_lifetime_,
        .color = emitter.color_
      };
      emitter.particles_.push_back(p);
    }
    
    // Update and render particles
    std::vector<sf::Vertex> vertices;
    
    emitter.particles_.erase(
      std::remove_if(emitter.particles_.begin(), emitter.particles_.end(),
        [&](Particle& p) {
          p.lifetime_ -= delta_time;
          p.position_ += p.velocity_ * delta_time;
          
          if (p.lifetime_ <= 0) return true;
          
          // Add quad vertices for this particle
          // (simplified; real implementation varies)
          sf::Color color = p.color_;
          color.a = static_cast<sf::Uint8>(255 * (p.lifetime_ / emitter.max_lifetime_));
          
          vertices.push_back(sf::Vertex(p.position_, color));
          // ... add 3 more vertices for the quad
          
          return false;
        }),
      emitter.particles_.end());
    
    // Render all particles
    if (!vertices.empty()) {
      VertexArrayParams params{
        .vertices = vertices,
        .indices = {},
        .texture_key = emitter.texture_key_,
        .shader_key = ""
      };
      render_context.DrawVertexArray(params);
    }
  }
}
```

### Performance Optimization

- **Batching:** Render all particles from an emitter in a single draw call.
- **Object Pooling:** Pre-allocate particle arrays to avoid frequent allocation.
- **Culling:** Skip emitters outside the camera view.
- **GPU Particles:** For high-count emitters, consider a compute shader (advanced).

## System Execution Order

During the game loop:

```
1. InitializeDrawableStaticSystem (once, at startup)
2. InitializeAnimationSystem (once, at startup)
3. [Main game loop]:
   a. AnimationSystem (update frames)
   b. DrawableSystem (render sprites)
   c. DrawTextSystem (render text)
   d. ParticleEmitterSystem (render particles)
   e. render_context.Display() (present frame)
```

See [Lifecycle](./lifecycle.md) for the complete initialization sequence.

## Testing Rendering Systems

Use a mock `IRenderContext` to test systems without SFML:

```cpp
class MockRenderContext : public IRenderContext {
 public:
  std::vector<DrawSpriteParams> draw_calls;
  
  void DrawSprite(const DrawSpriteParams& params) override {
    draw_calls.push_back(params);
  }
  
  // Mock implementations for other methods...
};

TEST(DrawableSystemTest, RendersVisibleEntities) {
  auto registry = CreateTestRegistry();
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{100, 200}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"sprite", {0, 0}, 0, 0, {1, 1}, true, ""});
  
  MockRenderContext mock;
  DrawableSystem(registry, mock, 0.016f);
  
  EXPECT_EQ(mock.draw_calls.size(), 1);
  EXPECT_EQ(mock.draw_calls[0].texture_key, "sprite");
}
```

## References

- [IRenderContext API](./irendercontext.md)
- [SFML Backend](./sfml-backend.md)
- [Lifecycle](./lifecycle.md)
- [Architecture](../architecture.md)
