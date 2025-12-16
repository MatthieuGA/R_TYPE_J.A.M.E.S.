# Controllable Player System

**Source:** `client/engine/systems/SystemsFunctions/ControllablePlayerSystem.cpp`

**Purpose:** Convert player inputs into accelerations applied to `Velocity`, respecting a ramp time to reach max speed.

**Components used:**

- `Inputs`
- `Controllable` (marks whether the entity is controllable)
- `Velocity`
- `PlayerTag` (contains `speed_max`)

## Behavior

- Compute target speed from inputs (`input.horizontal * speed`).
- Compute required acceleration to reach target speed in `time_to_max` seconds.
- Clamp acceleration between `-max_accel` and `+max_accel`.

## Main signature

```cpp
void ControllablePlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs,
    Eng::sparse_array<Com::Controllable> const &controllables,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PlayerTag> const &playerTags);
```

## Notes

- `time_to_max` is set to `0.15f` in current implementation.
- This system only updates accelerations; velocity is applied by `MovementSystem`.
