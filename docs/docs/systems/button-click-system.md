---
sidebar_position: 4
---

# Button Click System

**Source:** `client/engine/systems/systems_functions/ButtonClickSystem.cpp`

**Purpose:** Detect mouse hover and click on clickable entities, fire their callbacks, and update their visual color state.

---

## Components Used

| Component | Access | Description |
|-----------|--------|-------------|
| `HitBox` | Read | Size and scaling behavior |
| `Clickable` | Read/Write | Hover/click state, colors, callback |
| `Drawable` | Write | Sprite color updated |
| `Transform` | Read | Position, scale, origin |

---

## Dependencies

| Dependency | Type | Description |
|------------|------|-------------|
| `GameWorld` | Reference | Access to InputManager and window |
| `InputManager` | Via GameWorld | Mouse button state queries |

---

## Behavior

1. **Focus Check**: Returns immediately if the window doesn't have focus
2. **Mouse Button Query**: Uses `InputManager::IsMouseButtonPressed()` for backend-agnostic mouse state
3. **Hit Detection**: For each clickable entity:
   - Converts mouse position to world coords via `window_.mapPixelToCoords`
   - Builds the clickable rectangle considering transform scale and origin
   - Sets `isHovered` when pointer is inside the rectangle
4. **Click Detection**: Fires on button release after being pressed on the same entity
5. **Visual Feedback**: Updates `drawable.color` based on state priority: `clickColor` > `hoverColor` > `idleColor`

---

## Function Signature

```cpp
void ButtonClickSystem(Engine::registry &reg, GameWorld &game_world,
    Engine::sparse_array<Component::HitBox> &hit_boxes,
    Engine::sparse_array<Component::Clickable> &clickables,
    Engine::sparse_array<Component::Drawable> &drawables,
    Engine::sparse_array<Component::Transform> &transforms);
```

---

## Implementation Notes

The system uses the InputManager abstraction for mouse queries:

```cpp
// Query mouse button through InputManager (backend-agnostic)
const bool is_left_pressed =
    game_world.input_manager_->IsMouseButtonPressed(
        Engine::Input::MouseButton::Left);
```

This ensures the button click system works regardless of the underlying input backend (SFML, SDL, etc.).

---

## Notes

- **Backend Agnostic**: Uses `InputManager` for mouse state, not `sf::Mouse` directly
- **Click fires on release**: Not on press, for better UX
- **Colors applied directly**: No shader required for visual feedback

---

## Related Documentation

- [Input Abstraction Layer](../input-abstraction.md) - Mouse button abstraction
- [Input System](input-system.md) - Keyboard input handling
