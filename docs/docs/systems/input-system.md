# Input System

**Source:** `client/engine/systems/SystemsFunctions/InputSystem.cpp`

**Purpose:** Read keyboard state and populate the `Inputs` component.

**Components used:**

- `Inputs` (horizontal, vertical, shoot)

## Behavior

- Reset `horizontal`, `vertical`, and `shoot` to 0/false at the start of the pass.
- Read keys (local mapping):
  - `Q` = left (-1.0f)
  - `D` = right (+1.0f)
  - `Z` = up (-1.0f)
  - `S` = down (+1.0f)
  - `Space` = shoot (`shoot = true`)

## Main signature

```cpp
void InputSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs);
```

## Notes

- Uses `sf::Keyboard::isKeyPressed` and runs on the client.
- Key remapping requires code changes if needed.
