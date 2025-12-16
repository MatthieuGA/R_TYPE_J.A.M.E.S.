# Animation Enter Player System

**Source file:** `client/engine/systems/systems_functions/AnimationEnterPlayerSystem.cpp`

**Purpose:** Move players from off-screen into the play area, then enable their controls once they reach the target position.

**Components used:**

- `AnimationEnterPlayer` (entry flag)
- `Transform` (current position)
- `Velocity` (applied speed during entry)
- `PlayerTag` (`speed_max`, `isInPlay`)
- `Controllable` (added when entry is complete)

## Behavior

- While `animation_enter_player.isEntering` is true, sets `velocity.vx` to `player_tag.speed_max` to push the entity into the screen.
- When `transform.x` reaches or exceeds `75.0f`, clamps the position, clears `isEntering`, sets `player_tag.isInPlay` to true, and adds `Controllable`.
- Removes `AnimationEnterPlayer` after completion to avoid re-triggering.

## Main signature

```cpp
void AnimationEnterPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::AnimationEnterPlayer> &animation_enter_players);
```

## Notes

- The system does not use `dt`; velocity is later integrated by `MovementSystem`.
- `75.0f` is the default X entry offset.
