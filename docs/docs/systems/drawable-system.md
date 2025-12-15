# Drawable System

x**Source:** `client/engine/systems/systems_functions/render/DrawableSystem.cpp`

**Purpose:** Render entities with `Drawable` components: load textures via `RenderingEngine`, position sprites, and draw them using the plugin API.

**Components used:**

- `Drawable`
- `Transform`
- `Shader` (optional)
- `AnimatedSprite` (optional)
- `ParticleEmitter` (optional)

## Behavior

- Initialize textures not yet loaded via `game_world.rendering_engine_->LoadTexture()` (`InitializeDrawable`).
- Collect all drawable entities and particle emitters.
- Sort by `z_index` (and optionally texture ID) to honor draw order and optimize batching.
- Calculate world position with hierarchical transforms and animation offsets.
- Perform frustum culling to skip off-screen entities (improves performance).
- For each visible entity:
  - Calculate origin offset via `GetOffsetFromTransform`.
  - Apply position, scale, rotation, and opacity.
  - Render sprite using `rendering_engine_->RenderSprite()` (plugin-agnostic).
  - Apply shader uniforms if shader component present.
- Render particle emitters in batch via `rendering_engine_->RenderParticles()`.

## Main signature

```cpp
void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    Eng::sparse_array<Com::ParticleEmitter> &emitters);
```

## Notes

- Texture load failures are logged to `std::cerr`.
- Uses `game_world.rendering_engine_` (plugin abstraction) instead of direct SFML window access.
- Supports hierarchical transforms with proper world position calculation.
- Includes frustum culling and render statistics tracking.
- Compatible with any `IVideoModule` plugin (SFML, SDL, etc.).
