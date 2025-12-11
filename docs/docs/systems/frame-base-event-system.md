# Frame Base Event System

**Source file:** `client/engine/systems/systems_functions/FrameBaseEventSystem.cpp`

**Purpose:** Trigger callbacks bound to specific animation frames on an entity.

**Components used:**

- `FrameEvents` (list of frame events with target animation, trigger frame, callback, `triggered` flag)
- `AnimatedSprite` (current animation/frame)
- `Transform` (iterated alongside, not modified)

## Behavior

- For each entity with `FrameEvents`, fetches the current animation from `AnimatedSprite`.
- For each `FrameEvent`: if animation and current frame match and the event is not yet triggered, executes `action(entityId)` and marks `triggered`.
- Resets `triggered` when the animation frame returns to 0, allowing the event to fire again on the next loop.

## Main signature

```cpp
void FrameBaseEventSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::FrameEvents> &frame_events);
```

## Notes

- No `dt` dependency: triggering is solely based on animation frame progression.
- Events replay every loop because `triggered` is cleared when the frame cycles back to 0.
