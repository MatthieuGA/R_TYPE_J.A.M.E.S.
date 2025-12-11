# Parallax System

**Source:** `client/engine/systems/SystemsFunctions/ParallaxSystem.cpp`

**Purpose:** Scroll parallax background layers.

**Components used:**

- `ParrallaxLayer` (scroll_speed)
- `Transform`
- `Drawable` (for texture size)

## Behavior

- Advance the layer X position by `scroll_speed * dt`.
- Reset X position when it exceeds the negative texture width to create a loop.

## Main signature

```cpp
void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables);
```

## Notes

- Relies on `game_world.window_size_` and `game_world.last_delta_`.
- Checks `drawable.isLoaded` before using texture size.
