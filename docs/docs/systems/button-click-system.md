# Button Click System

**Source file:** `client/engine/systems/systems_functions/ButtonClickSystem.cpp`

**Purpose:** Detect mouse hover and click on clickable entities, fire their callbacks, and update their visual color state.

**Components used:**

- `HitBox` (size, optional scaling with `Transform`)
- `Clickable` (`isHovered`, `isClicked`, colors, `onClick` callback)
- `Drawable` (sprite color updated)
- `Transform` (position, scale, origin)

## Behavior

- Converts mouse position to world coords via `window_.mapPixelToCoords`.
- Builds the clickable rectangle considering `transform.scale` if `scaleWithTransform` is true and origin from `GetOffsetFromTransform`.
- Sets `isHovered` when the pointer is inside the rectangle.
- Detects left-button press→release to invoke `clickable.onClick` if present.
- Updates `drawable.color` based on state priority: `clickColor` > `hoverColor` > `idleColor`.

## Main signature

```cpp
void ButtonClickSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::HitBox> &hit_boxes,
    Eng::sparse_array<Com::Clickable> &clickables,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Transform> &transforms);
```

## Notes

- Click fires on button release after being pressed on the same entity.
- Colors are applied directly to `drawable.color`; no shader is required.
