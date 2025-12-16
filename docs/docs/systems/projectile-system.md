# Projectile System

**Source:** `client/engine/systems/SystemsFunctions/ProjectileSystem.cpp`

**Purpose:** Update projectile positions and remove those leaving the play area.

**Components used:**

- `Transform`
- `Projectile`

## Behavior

- Move projectile X position by `projectile.speed * dt`.
- If the projectile leaves an expanded rectangle around the window (`window_size_` ± 100px), mark it for removal (`reg.KillEntity`).

## Main signature

```cpp
void ProjectileSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Projectile> &projectiles);
```

## Notes

- Collect entities to remove first, then call `KillEntity` to avoid invalidating zippers during iteration.
