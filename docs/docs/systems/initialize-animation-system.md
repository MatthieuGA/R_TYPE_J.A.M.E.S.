# Initialize Drawable Animated

**Fichier source:** `client/engine/systems/SystemsFunctions/InitializeAnimationSystem.cpp`

**But:** Initialiser les `Drawable` des entités qui ont des `AnimatedSprite` (charger la texture, définir l'origine et la région de la frame initiale).

**Composants utilisés:**

- `AnimatedSprite`
- `Drawable`
- `Transform`

## Comportement

- Charge la texture depuis `drawable.spritePath` si elle n'est pas encore chargée.
- Définit l'origine du sprite en tenant compte de l'animation (taille de frame et transform).
- Définit le `IntRect` du sprite à la frame courante.

## Signature principale

```cpp
void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites);
```

## Notes

- Affiche un message d'erreur sur `std::cerr` si le chargement échoue.
