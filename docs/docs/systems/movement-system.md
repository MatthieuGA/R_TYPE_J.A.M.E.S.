# Movement System

**Source:** `client/engine/systems/SystemsFunctions/MovementSystem.cpp`

**Purpose:** Apply velocity and acceleration to entity `Transform`s.

**Components used:**

- `Transform` (x, y, rotation, scale)
- `Velocity` (vx, vy, accelerationX, accelerationY)

## Behavior

- Update position: `transform.x += velocity.vx * dt` and `transform.y += velocity.vy * dt`.
- Update velocity by applying acceleration: `vx += accelerationX * dt`.

## Main signature

```cpp
void MovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities);
```

## Notes

- `dt` typically comes from `game_world.last_delta_`.
- No collision or bounds handling here; this is simple kinematic motion.
