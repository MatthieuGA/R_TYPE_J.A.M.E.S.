# Playfield Limit System

**Fichier source:** `client/engine/systems/SystemsFunctions/PlayfieldLimitSystem.cpp`

**But:** Empêcher les entités (typiquement le joueur) de sortir de la fenêtre de jeu.

**Composants utilisés:**
- `Transform`
- `PlayerTag`

## Comportement

- Clamp les coordonnées `x` et `y` entre `0` et la taille de la fenêtre (`window.getSize()`).
- Agit directement sur `Transform` pour corriger les positions hors-limites.

## Signature principale

```cpp
void PlayfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags);
```

## Notes
- Utilise la taille de la fenêtre SFML fournie comme argument.
- Cette logique est simple et peut être étendue (marges, wrap, etc.).
