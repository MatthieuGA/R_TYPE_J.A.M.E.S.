# Initialize Drawable Animated System

**Source:** `client/engine/systems/SystemsFunctions/InitializeAnimationSystem.cpp`

**Purpose:** Initialize `Drawable` components for entities that have an `AnimatedSprite` by loading the texture, setting the origin, and configuring the initial frame region.

**Components used:**

- `AnimatedSprite`
- `Drawable`
- `Transform`

## Behavior

- Load the texture from `drawable.spritePath` if not already loaded.
- Set the sprite origin accounting for animation frame size and the transform.
- Set the sprite `IntRect` to the current frame.

## Main signature

```cpp
void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites);
```

## Notes

- Logs an error to `std::cerr` if texture loading fails.
