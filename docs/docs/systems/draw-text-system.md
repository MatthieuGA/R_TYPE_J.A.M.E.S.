# Draw Text System

**Source:** `client/engine/systems/SystemsFunctions/Render/DrawTextSystem.cpp`

**Purpose:** Render entities with a `Text` component while applying hierarchical transforms (position, rotation, scale) and respecting draw order via `z_index`.

**Components used:**

- `Transform`
- `Text`

## Behavior

- Load the font from `Text::fontPath` on first pass (`InitializeText`).
- Compute the text origin via `GetOffsetFromTransform` to align with the `Transform` origin.
- Apply local offsets (`offset`), parent hierarchy transforms, and local rotation.
- Apply cumulative scale (`CalculateCumulativeScale`) and adjust opacity via `Text::opacity`.
- Sort entities by `z_index` before drawing to guarantee order.

## Main signature

```cpp
void DrawTextRenderSystem(
    Eng::registry &reg,
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts);
```

## Notes

- `InitializeText` sets the text size to `characterSize * 10` then applies cumulative scale divided by 10 to stay consistent with sprites.
- If font loading fails, an error is logged to `std::cerr` but `is_loaded` flips to `true` to avoid endless retries.
- The `offset` field lets you locally shift text relative to the `Transform` without changing hierarchy.
