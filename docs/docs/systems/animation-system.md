# Animation System

**Source file:** `client/engine/systems/SystemsFunctions/AnimationSystem.cpp`

**Purpose:** Update animated sprites over time and advance frames.

**Components used:**

- `AnimatedSprite` (animation state, frames, duration)
- `Drawable` (sprite, texture)

## Behavior

- Computes the texture rect to display (`SetFrame`) based on current frame and frame dimensions.
- Advances the frame when elapsed time exceeds `frameDuration` (`NextFrame`).
- Honours `loop`: either wraps or stays on the last frame.
- If the texture is not ready or animation is disabled, the frame rect is recomputed but frame advancement is skipped.

## Main signature

```cpp
void AnimationSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
    Eng::sparse_array<Com::Drawable> &drawables);
```

## Notes

- Uses `dt` (delta time) to accumulate `elapsedTime`.
- Supports multi-row/column spritesheets.
