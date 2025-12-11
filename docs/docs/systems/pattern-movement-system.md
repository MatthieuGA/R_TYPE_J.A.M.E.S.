# Pattern Movement System

**Source file:** `client/engine/systems/systems_functions/PaternMovementSystem.cpp`

**Purpose:** Apply movement patterns to entities (enemies, special projectiles) according to the configured `PatternMovement` type.

**Components used:**

- `PatternMovement` (pattern type, base speed/dir, amplitudes, frequencies, waypoints, follow target, radius, elapsed time)
- `Transform` (position; sometimes written directly for circular motion)
- `Velocity` (resulting velocity consumed by `MovementSystem`)

## Behavior

- Increments `patternMovement.elapsed` with `dt`, then dispatches to the function matching `PatternType`:
  - **Straight**: sets `vx/vy = baseDir * baseSpeed`; kills entity if far outside bounds (x<-100/x>2000/y<-100/y>1200).
  - **SineHorizontal / ZigZagHorizontal**: advances in X with base speed and oscillates in Y (continuous sine or sign-based zigzag).
  - **SineVertical / ZigZagVertical**: advances in Y with base speed and oscillates in X.
  - **Wave**: combines sine offsets on X and Y added to base velocity.
  - **Waypoints**: moves toward each waypoint until within `waypointThreshold`, then cycles.
  - **FollowPlayer**: finds the nearest player if `targetEntityId == -1`, then steers toward it; resets if the target vanishes.
  - **Circular**: writes `transform` to follow a circle around `spawnPos` using `radius` and `baseSpeed` (`elapsed` drives the angle).
- Each pattern may kill the entity if it exits extended bounds.

## Main signature

```cpp
void PaternMovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PatternMovement> &patern_movements);
```

## Notes

- Relies on `MovementSystem` to integrate velocities (except `Circular`, which writes position directly).
- Hardcoded kill bounds prevent off-screen accumulation.
- Player search uses `PlayerTag` and may throw; exceptions are caught locally.
