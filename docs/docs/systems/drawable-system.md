# Drawable System

**Source:** `client/engine/systems/SystemsFunctions/DrawableSystem.cpp`

**Purpose:** Render entities with `Drawable` components: load textures, position sprites, and draw them.

**Components used:**

- `Drawable`
- `Transform`
- `Shader` (optional)

## Behavior

- Initialize textures not yet loaded from `drawable.spritePath` (`InitializeDrawable`).
- Compute the sprite origin via `GetOffsetFromTransform` and apply it.
- Sort entities by `z_index` to honor draw order.
- For each entity, update position, scale, rotation, and opacity, then draw the sprite on `game_world.window_`.
- If a shader is attached and loaded, apply it and update the `time` uniform.

## Main signature

```cpp
void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
	Eng::sparse_array<Com::Transform> const &transforms,
	Eng::sparse_array<Com::Drawable> &drawables,
	Eng::sparse_array<Com::Shader> &shaders);
```

## Notes

- Texture load failures are logged to `std::cerr`.
- Depends on `game_world` for `window_` and `total_time_clock_`.
