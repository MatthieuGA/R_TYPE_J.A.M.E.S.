# Collision Detection System

**Fichier source:** `client/engine/systems/SystemsFunctions/CollisionDetectionSystem.cpp`

**But:** Détecter les collisions entre hitboxes et résoudre les pénétrations (séparation) pour les entités solides.

**Composants utilisés:**
- `Transform`
- `HitBox`
- `Solid`
- `GameWorld` (contexte)

## Comportement

- Calcul des offsets d'origine via `GetOffsetFromTransform`.
- Test AABB (axis-aligned bounding box) pour déterminer l'intersection entre deux hitboxes (`IsCollidingFromOffset`).
- Si collision, calcule la pénétration en X et Y et sépare les entités le long de l'axe de pénétration le plus faible.
- Gère des flags `isSolid` et `isLocked` pour savoir quelles entités peuvent être déplacées pour résoudre la collision.

## Signature principale

```cpp
void CollisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids);
```

## Notes
- La résolution de collision prend en compte l'échelle (`trans.scale`) et les offsets calculés.
- Si les deux entités sont `isLocked`, la collision n'est pas résolue (aucun mouvement).
- Si les deux entités sont solides et mobiles, la séparation peut être partagée (implémentation selon la logique interne).
