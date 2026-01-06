---
sidebar_position: 16
---

# Input System

**Source:** `client/engine/systems/systems_functions/InputSystem.cpp`

**Purpose:** Read logical action state from the InputManager and populate the `Inputs` component for player-controlled entities.

---

## Overview

The Input System bridges the gap between the [Input Abstraction Layer](../input-abstraction.md) and the ECS. It queries the `InputManager` for action states and updates `Inputs` components accordingly.

This system uses **logical actions** (e.g., `MoveLeft`, `Shoot`) instead of physical keys, making the game code independent of the underlying input backend (SFML, SDL, etc.).

---

## Components Used

| Component | Access | Description |
|-----------|--------|-------------|
| `Inputs` | Write | Stores movement direction and shoot state |

---

## Dependencies

| Dependency | Type | Description |
|------------|------|-------------|
| `InputManager<Game::Action>` | Reference | Provides action state queries |

---

## Behavior

1. **Focus Check**: Returns immediately if the window doesn't have focus
2. **Action Queries**: For each entity with `Inputs`:
   - Uses `GetAxis()` for smooth movement values (-1.0 to +1.0)
   - Uses `IsActionActive()` for discrete actions (shoot)
3. **State Tracking**: Preserves `last_shoot_state` for edge detection

### Action to Input Mapping

| Component Field | Negative Action | Positive Action |
|-----------------|-----------------|-----------------|
| `horizontal` | `MoveLeft` | `MoveRight` |
| `vertical` | `MoveUp` | `MoveDown` |
| `shoot` | - | `Shoot` |

---

## Function Signature

```cpp
void InputSystem(
    Engine::registry &reg,
    Engine::Input::InputManager<Game::Action> &input_manager,
    Engine::sparse_array<Component::Inputs> &inputs);
```

### Parameters

| Parameter | Description |
|-----------|-------------|
| `reg` | Engine registry (unused but required for system signature) |
| `input_manager` | The input manager configured with game bindings |
| `inputs` | Sparse array of Inputs components to update |

---

## Implementation

```cpp
void InputSystem(Engine::registry &reg,
    Engine::Input::InputManager<Game::Action> &input_manager,
    Engine::sparse_array<Component::Inputs> &inputs) {
    
    // Ignore input when window loses focus
    if (!input_manager.HasFocus())
        return;

    for (auto &&[i, input] : make_indexed_zipper(inputs)) {
        // Track previous shoot state for edge detection
        input.last_shoot_state = input.shoot;

        // Use logical actions via GetAxis helper
        // Returns -1.0, 0.0, or +1.0 based on action state
        input.horizontal = input_manager.GetAxis(
            Game::Action::MoveLeft, 
            Game::Action::MoveRight);
        
        input.vertical = input_manager.GetAxis(
            Game::Action::MoveUp, 
            Game::Action::MoveDown);

        // Discrete action for shooting
        input.shoot = input_manager.IsActionActive(Game::Action::Shoot);
    }
}
```

---

## Default Key Bindings

The InputManager is configured with these default bindings (set in `GameInputBindings.hpp`):

### Movement

| Action | Primary | Secondary | Tertiary |
|--------|---------|-----------|----------|
| MoveUp | `Z` | `W` | `↑` Arrow |
| MoveDown | `S` | - | `↓` Arrow |
| MoveLeft | `Q` | `A` | `←` Arrow |
| MoveRight | `D` | - | `→` Arrow |

### Combat

| Action | Primary | Secondary |
|--------|---------|-----------|
| Shoot | `Space` | Left Mouse Button |

---

## Network Integration

The `Inputs` component is converted to a bitfield for network transmission:

```cpp
uint8_t InputToBitfield(const Component::Inputs &input) {
    uint8_t bitfield = 0;

    if (input.vertical < 0.0f)   bitfield |= (1 << 0);  // Up
    if (input.vertical > 0.0f)   bitfield |= (1 << 1);  // Down
    if (input.horizontal < 0.0f) bitfield |= (1 << 2);  // Left
    if (input.horizontal > 0.0f) bitfield |= (1 << 3);  // Right
    if (input.shoot)             bitfield |= (1 << 4);  // Shoot

    return bitfield;
}
```

This follows the RFC Section 6.1 (PLAYER_INPUT) specification.

---

## Notes

- **No Physical Keys**: This system never checks `sf::Keyboard` directly
- **Focus Handling**: Prevents unintended input when alt-tabbed
- **Backend Agnostic**: Works with any `IInputBackend` implementation
- **Key Remapping**: Change bindings at runtime via `InputManager::BindKey()`

---

## Related Documentation

- [Input Abstraction Layer](../input-abstraction.md) - Full architecture documentation
- [Button Click System](button-click-system.md) - UI input handling
- [Protocol RFC](../network-rfc.txt) - Network input packet format
