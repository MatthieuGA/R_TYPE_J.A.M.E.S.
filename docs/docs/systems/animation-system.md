# Animation System

**Fichier source:** `client/engine/systems/SystemsFunctions/animationSystem.cpp`

**But:** Mettre à jour les sprites animés en fonction du temps et faire avancer les frames.

**Composants utilisés:**
- `AnimatedSprite` (animation state, frames, durée)
- `Drawable` (sprite, texture)

## Comportement

- Calcule la cellule de texture à afficher (`SetFrame`) en fonction de la frame courante et de la largeur/hauteur d'une frame.
- Avance la frame lorsque le temps écoulé dépasse `frameDuration` (`NextFrame`).
- Respecte la propriété `loop` pour boucler ou rester sur la dernière frame.
- Si la texture n'est pas prête ou l'animation est désactivée, la frame est recalculée mais l'avancement n'est pas effectué.

## Signature principale

```cpp
void AnimationSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
    Eng::sparse_array<Com::Drawable> &drawables);
```

## Notes
- Le système utilise `dt` (delta time) pour accumuler `elapsedTime`.
- Prend en charge plusieurs frames par sprite-sheet (colonnes/rows).
