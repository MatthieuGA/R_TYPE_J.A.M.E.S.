# Shoot System

**Fichier source:** `client/Engine/Systems/SytemsFunctions/shootSystem.cpp`

**But:** Gérer la mécanique de tir du joueur et créer des entités projectile.

**Composants utilisés:**
- `Transform`
- `Inputs`
- `PlayerTag`

## Comportement

- Pour chaque joueur, décrémente `shoot_cooldown` selon `game_world.last_delta_`.
- Si l'entrée `shoot` est active et que le cooldown est écoulé, crée une nouvelle entité projectile via `createProjectile` et remet le cooldown à `shoot_cooldown_max`.
- `createProjectile` ajoute les composants `Transform`, `Drawable`, `AnimatedSprite` et `Projectile` à la nouvelle entité.

## Signature principale

```cpp
void ShootSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::PlayerTag> &player_tags);
```

## Notes
- Le projectile est initialisé avec des valeurs par défaut (vitesse, sprite, etc.) codées en dur dans `createProjectile`.
- La fonction `createProjectile` utilise `reg.SpawnEntity()` puis `reg.AddComponent<...>`.
