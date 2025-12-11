# Collision Detection System

**Source:** `client/engine/systems/SystemsFunctions/CollisionDetectionSystem.cpp`

**Purpose:** Detect collisions between hitboxes and resolve penetrations (separation) for solid entities.

**Components used:**

- `Transform`
- `HitBox`
- `Solid`
- `GameWorld` (context)

## Behavior

- Compute origin offsets via `GetOffsetFromTransform`.
- Perform AABB tests to detect intersection between two hitboxes (`IsCollidingFromOffset`).
- On collision, compute penetration in X and Y and separate along the smallest penetration axis.
- Use `isSolid` and `isLocked` flags to decide which entities can be moved to resolve the collision.

## Main signature

```cpp
void CollisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids);
```

## Notes

- Collision resolution accounts for scale (`trans.scale`) and computed offsets.
- If both entities are `isLocked`, no resolution occurs.
- If both are solid and movable, separation can be shared (implementation-specific).
