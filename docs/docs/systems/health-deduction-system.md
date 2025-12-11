# Health Deduction System

**Source file:** `client/engine/systems/systems_functions/HealthDeductionSystem.cpp`

**Purpose:** Handle projectile/target collisions, apply damage, trigger hit/death animations, and clean up destroyed entities.

**Components used:**

- `Health` (current hit points)
- `HealthBar` (damage timer refresh)
- `AnimatedSprite` ("Hit" and "Death" animations)
- `HitBox` (AABB collision)
- `Transform` (positions for AABB test)
- `Projectile` (damage, ownership)
- `PlayerTag` / `EnemyTag` (friend/foe filtering)
- `TimedEvents`, `FrameEvents`, `PatternMovement` (removed on death)

## Behavior

- For each entity with `Health`, iterates projectiles and tests collision via `IsColliding` (AABB).
- Skips friendly fire based on `projectile.isEnemyProjectile` against Player/Enemy tags.
- On impact:
  - Decrements `health.currentHealth` by `projectile.damage`.
  - Resets `health_bar.timer_damage` to 0 if present.
  - Plays "Hit" animation and forces frame 1 if `AnimatedSprite` exists.
  - Removes `Projectile` from the projectile entity; if it has a sprite, plays "Death" and adds `AnimationDeath`, otherwise kills it.
- If `health.currentHealth <= 0`, invokes `DeathHandling`: adds `AnimationDeath`, removes `Health`, `HitBox`, Player/Enemy tags, `TimedEvents`, `FrameEvents`, `PatternMovement`, then plays "Death" if available or kills the entity.

## Main signature

```cpp
void HealthDeductionSystem(Eng::registry &reg,
    Eng::sparse_array<Component::Health> &healths,
    Eng::sparse_array<Component::HealthBar> &health_bars,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::HitBox> const &hitBoxes,
    Eng::sparse_array<Component::Transform> const &transforms,
    Eng::sparse_array<Component::Projectile> const &projectiles);
```

## Notes

- AABB checks include offsets/scales via `IsColliding` (see `CollidingTools`).
- "Hit"/"Death" handling assumes non-looping projectile animations.
- Entities with no collision or remaining health are unaffected.
