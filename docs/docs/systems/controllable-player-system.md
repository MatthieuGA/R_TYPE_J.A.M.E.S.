# Controllable Player System

**Fichier source:** `client/engine/systems/SystemsFunctions/ControllablePlayerSystem.cpp`

**But:** Convertir les entrées du joueur en accélérations appliquées à la composante `Velocity`, en respectant un temps d'accélération pour atteindre la vitesse maximale.

**Composants utilisés:**
- `Inputs`
- `Controllable` (indique si l'entité est contrôlable)
- `Velocity`
- `PlayerTag` (contient `speed_max`)

## Comportement

- Calcule la vitesse cible à partir des entrées (`input.horizontal * speed`).
- Calcule l'accélération requise pour atteindre la vitesse cible en `time_to_max` secondes.
- Clamp l'accélération entre `-max_accel` et `+max_accel`.

## Signature principale

```cpp
void ControllablePlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs,
    Eng::sparse_array<Com::Controllable> const &controllables,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PlayerTag> const &playerTags);
```

## Notes
- `time_to_max` est fixé à `0.15f` dans l'implémentation actuelle.
- Le système ne met à jour que les accélérations; la vélocité effective est appliquée par le `MovementSystem`.
