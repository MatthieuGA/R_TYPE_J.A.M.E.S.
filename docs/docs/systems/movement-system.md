# Movement System

**Fichier source:** `client/engine/systems/SystemsFunctions/MovementSystem.cpp`

**But:** Appliquer la vélocité et l'accélération aux `Transform` des entités.

**Composants utilisés:**

- `Transform` (x, y, rotation, scale)
- `Velocity` (vx, vy, accelerationX, accelerationY)

## Comportement

- Met à jour la position : `transform.x += velocity.vx * dt` et `transform.y += velocity.vy * dt`.
- Met à jour la vitesse en appliquant l'accélération: `vx += accelerationX * dt`.

## Signature principale

```cpp
void MovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities);
```

## Notes

- `dt` provient typiquement du `game_world.last_delta_`.
- Ce système ne gère pas de collision/limit; il applique juste la physique kinematique simple.
