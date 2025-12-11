# Player Hit System

**Source file:** `client/engine/systems/systems_functions/PlayerHitSystem.cpp`

**Purpose:** Reset the player's animation back to the default after the hit animation completes.

**Components used:**

- `PlayerTag` (filters affected entities)
- `AnimatedSprite` (current animation and elapsed time)
- `GameWorld` (`last_delta_` for timing)

## Behavior

- Iterates entities with `PlayerTag` and `AnimatedSprite`.
- If current animation is "Hit", increments `elapsedTime` by `game_world.last_delta_`.
- When `elapsedTime` exceeds the current animation's `frameDuration`, switches to "Default" and resets `elapsedTime` to 0.

## Main signature

```cpp
void PlayerHitSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::PlayerTag> &player_tags);
```

## Notes

- Assumes "Hit" is non-looping and has at least one frame.
- Only affects players; other entities rely on `DeathAnimationSystem` or their own systems.
