# Timed Event System

**Source file:** `client/engine/systems/systems_functions/TimedEventSystem.cpp`

**Purpose:** Execute cooldown-based periodic actions for each entity.

**Components used:**

- `TimedEvents` (list of `CooldownAction` with `cooldown`, `cooldown_max`, and `action` callback)
- `GameWorld` (`last_delta_` used as time step)

## Behavior

- For each entity with `TimedEvents`, increments each action's `cooldown` by `game_world.last_delta_`.
- When `cooldown` exceeds `cooldown_max`, resets it to 0 and executes `action(entityId)` if set.
- Supports chaining multiple independent periodic actions on the same entity (e.g., auto-fire, timed effects).

## Main signature

```cpp
void TimedEventSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::TimedEvents> &timed_events);
```

## Notes

- No ordering guarantees beyond the vector order of `cooldown_actions`.
- Frame-rate independent thanks to delta time usage.
