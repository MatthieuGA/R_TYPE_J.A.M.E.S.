# Player System

**Source:** `client/engine/systems/SystemsFunctions/PlayerSystem.cpp`

**Purpose:** Update player animation based on vertical velocity and sync rotation of the player and its children based on horizontal velocity.

**Components used:**

- `PlayerTag` (contains `speed_max` and other player properties)
- `Velocity` (reads `vx` and `vy`)
- `AnimatedSprite` (updates `current_frame`)
- `Transform` (updates `rotationDegrees` for player and children)

## Behavior

### Vertical animation

`animated_sprite.current_frame` is set according to vertical velocity (`velocity.vy`):

| Condition | Frame | Animation |
|-----------|-------|-----------|
| `vy > 200.f` | 0 | Fast downward |
| `vy >= 75` | 1 | Moderate downward |
| `vy < -200.f` | 4 | Fast upward |
| `vy <= -75` | 3 | Moderate upward |
| Else | 2 | Neutral |

### Horizontal rotation

The system computes a rotation angle based on horizontal velocity:

```cpp
float rotation = velocity.vx / player_tag.speed_max * 5.f;
transform.rotationDegrees = rotation;
```

This gives a tilt proportional to horizontal speed.

### Child sync

Rotation propagates to all child entities:

```cpp
for (auto &&[j, child_transform] : make_indexed_zipper(transforms)) {
    if (child_transform.parent_entity.has_value() &&
        child_transform.parent_entity.value() == i) {
        child_transform.rotationDegrees = rotation;
    }
}
```

Child assets (e.g., the charge effect) follow the same tilt.

## Main signature

```cpp
void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);
```

## Notes

- Animation thresholds (200.f, 75) are hard-coded in current logic.
- The rotation factor (5.f) sets the max tilt amplitude.
- This system does not apply physics; it only syncs visuals to velocity.
- Child lookup scans all transforms; could be optimized with `Transform.children` (like `ChargingShowAssetPlayerSystem`).

## Possible improvements

1. **Optimization:** Use `transform.children` instead of scanning all transforms for children.
2. **Configuration:** Externalize animation thresholds and rotation factor into constants or `PlayerTag`.
3. **Interpolation:** Smoothly transition between animation frames.
4. **Vertical rotation:** Add slight pitch based on `vy` for more dynamism.
