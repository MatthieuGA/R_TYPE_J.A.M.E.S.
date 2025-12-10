# Player System

**Fichier source:** `client/engine/systems/SystemsFunctions/PlayerSystem.cpp`

**But:** Mettre à jour l'état d'animation du joueur en fonction de sa vélocité verticale et synchroniser la rotation du joueur et de ses enfants en fonction de la vélocité horizontale.

**Composants utilisés:**

- `PlayerTag` (contient `speed_max` et autres propriétés de joueur)
- `Velocity` (lecture de `vx` et `vy`)
- `AnimatedSprite` (mise à jour de `current_frame`)
- `Transform` (mise à jour de `rotationDegrees` pour le joueur et ses enfants)

## Comportement

### Animation Verticale

Le système définit `animated_sprite.current_frame` selon la vélocité verticale (`velocity.vy`):

| Condition | Frame | Animation |
|-----------|-------|-----------|
| `vy > 200.f` | 0 | Mouvement rapide vers le bas |
| `vy >= 75` | 1 | Mouvement modéré vers le bas |
| `vy < -200.f` | 4 | Mouvement rapide vers le haut |
| `vy <= -75` | 3 | Mouvement modéré vers le haut |
| Sinon | 2 | Position neutre |

### Rotation Horizontale

Le système calcule un angle de rotation basé sur la vélocité horizontale:

```cpp
float rotation = velocity.vx / player_tag.speed_max * 5.f;
transform.rotationDegrees = rotation;
```

Cette rotation donne un effet d'inclinaison proportionnel à la vitesse de déplacement horizontal.

### Synchronisation des Enfants

Le système propage la rotation à toutes les entités enfants du joueur:

```cpp
for (auto &&[j, child_transform] : make_indexed_zipper(transforms)) {
    if (child_transform.parent_entity.has_value() &&
        child_transform.parent_entity.value() == i) {
        child_transform.rotationDegrees = rotation;
    }
}
```

Cela garantit que les assets attachés au joueur (comme l'effet de charge) suivent la même rotation.

## Signature principale

```cpp
void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);
```

## Notes

- Les seuils numériques pour l'animation (200.f, 75) sont codés en dur dans la logique actuelle.
- Le facteur de rotation (5.f) détermine l'amplitude maximale de l'inclinaison.
- Ce système n'applique pas de physique, il synchronise uniquement les visuels à la vélocité.
- La recherche d'enfants parcourt tous les transforms - pourrait être optimisé avec `Transform.children` (similaire au `ChargingShowAssetPlayerSystem`).

## Améliorations Potentielles

1. **Optimisation**: Utiliser `transform.children` au lieu de parcourir tous les transforms pour trouver les enfants.
2. **Configuration**: Externaliser les seuils d'animation et le facteur de rotation dans des constantes ou dans `PlayerTag`.
3. **Interpolation**: Ajouter une transition douce entre les frames d'animation.
4. **Rotation Verticale**: Ajouter une légère rotation verticale basée sur `vy` pour plus de dynamisme.
