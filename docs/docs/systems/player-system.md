# Player System

**Fichier source:** `client/Engine/Systems/SytemsFunctions/playerSystem.cpp`

**But:** Mettre à jour l'état d'animation du joueur en fonction de sa vélocité verticale.

**Composants utilisés:**
- `PlayerTag` (contient `speed_max` et autres propriétés de joueur)
- `Velocity`
- `AnimatedSprite`

## Comportement

- Définit `animated_sprite.currentFrame` selon `velocity.vy` pour choisir l'animation (vers le haut, bas, neutre).

## Signature principale

```cpp
void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);
```

## Notes
- Les seuils numériques (par ex. 200.f, 75) sont codés en dur dans la logique actuelle.
- Ce système n'applique pas de physique, il synchronise uniquement l'animation à la vélocité.
