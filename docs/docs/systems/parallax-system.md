# Parallax System

**Fichier source:** `client/Engine/Systems/SytemsFunctions/parallaxSystem.cpp`

**But:** Gérer le défilement parallax des couches d'arrière-plan.

**Composants utilisés:**
- `ParrallaxLayer` (scroll_speed)
- `Transform`
- `Drawable` (pour la taille de la texture)

## Comportement

- Avance la position X de la couche selon `scroll_speed * dt`.
- Réinitialise la position X lorsqu'elle dépasse la largeur négative de la texture, créant une boucle.

## Signature principale

```cpp
void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables);
```

## Notes
- Le système s'appuie sur `game_world.window_size_` et `game_world.last_delta_`.
- On vérifie `drawable.isLoaded` avant d'utiliser la taille de la texture.
