# Death Animation System

**Source file:** `client/engine/systems/systems_functions/DeathAnimationSystem.cpp`

**Purpose:** Remove entities marked for a death animation once their non-looping animation finishes.

**Components used:**

- `AnimationDeath` (death-in-progress flag)
- `AnimatedSprite` (current animation state)

## Behavior

- Iterates entities with `AnimationDeath`.
- If `AnimatedSprite` exists and `currentAnimation` returned to "Default" (death anim ended), kills the entity via `reg.KillEntity`.
- If no `AnimatedSprite`, removes the entity immediately.

## Main signature

```cpp
void DeathAnimationSystem(Eng::registry &reg,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::AnimationDeath> &animation_deaths);
```

## Notes

- Assumes `AnimationSystem` switches back to "Default" after a non-looping death animation.
- Loop safety is expected to be configured on the animation itself.
