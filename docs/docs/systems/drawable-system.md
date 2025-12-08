# Drawable System

**Fichier source:** `client/engine/systems/SystemsFunctions/drawableSystem.cpp`

**But:** Gérer le rendu des entités avec un `Drawable` : charger les textures, positionner et dessiner les sprites.

**Composants utilisés:**

-   `Drawable`
-   `Transform`
-   `Shader` (optionnel)

## Comportement

-   Initialise (`InitializeDrawable`) les textures non chargées depuis `drawable.spritePath`.
-   Calcule l'origine du sprite via `GetOffsetFromTransform` et l'applique.
-   Trie les entités par `z_index` pour respecter l'ordre de dessin.
-   Pour chaque entité, met à jour la position, l'échelle, la rotation et l'opacité, et dessine le sprite sur `game_world.window_`.
-   Si un shader est associé et chargé, il est appliqué et le `time` uniform est mis à jour.

## Signature principale

```cpp
void DrawableSystem(    Eng::registry &reg, GameWorld &game_world,    Eng::sparse_array<Com::Transform> const &transforms,    Eng::sparse_array<Com::Drawable> &drawables,    Eng::sparse_array<Com::Shader> &shaders);
```

## Notes

-   Les erreurs de chargement de texture sont loggées dans `std::cerr`.
-   Le système dépend du `game_world` pour le `window_` et la `total_time_clock_`.