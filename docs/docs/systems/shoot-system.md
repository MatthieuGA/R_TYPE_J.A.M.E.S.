# Shoot System

**Source:** `client/engine/systems/SystemsFunctions/ShootPlayerSystem.cpp`

**Purpose:** Handle player shooting mechanics and spawn projectile entities.

**Components used:**

- `Transform`
- `Inputs`
- `PlayerTag`

## Behavior

- For each player, decrement `shoot_cooldown` using `game_world.last_delta_`.
- If `shoot` input is active and cooldown elapsed, create a projectile via `CreateProjectile` and reset cooldown to `shoot_cooldown_max`.
- `CreateProjectile` adds `Transform`, `Drawable`, `AnimatedSprite`, and `Projectile` components to the new entity.

## Main signature

```cpp
void ShootPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::PlayerTag> &player_tags);
```

## Notes

- Projectile defaults (speed, sprite, etc.) are hard-coded in `CreateProjectile`.
- `CreateProjectile` uses `reg.SpawnEntity()` then `reg.AddComponent<...>`.
