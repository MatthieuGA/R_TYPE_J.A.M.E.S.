# Projectile System

**Fichier source:** `client/Engine/Systems/SytemsFunctions/projectileSystem.cpp`

**But:** Mettre à jour la position des projectiles et supprimer ceux qui sortent de la zone de jeu.

**Composants utilisés:**
- `Transform`
- `Projectile`

## Comportement

- Avance la position X du projectile selon `projectile.speed * dt`.
- Si le projectile sort d'un rectangle étendu autour de la fenêtre (`window_size_` ± 100px), il est marqué pour suppression (`reg.KillEntity`).

## Signature principale

```cpp
void ProjectileSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Projectile> &projectiles);
```

## Notes
- Le système collecte d'abord les entités à supprimer, puis appelle `KillEntity` pour éviter d'invalider les zippers en cours d'itération.
