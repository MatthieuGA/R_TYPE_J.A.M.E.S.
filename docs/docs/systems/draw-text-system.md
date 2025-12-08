# Draw Text System

**Fichier source:** `client/engine/systems/SystemsFunctions/Render/drawTextSystem.cpp`

**But:** Gerer le rendu des entites possedant un composant `Text` en appliquant les transformations hierarchiques (position, rotation, echelle) et en respectant l'ordre de dessin via `z_index`.

**Composants utilises:**

- `Transform`
- `Text`

## Comportement

- Charge la police depuis `Text::fontPath` au premier passage (`InitializeText`).
- Calcule l'origine du texte via `GetOffsetFromTransform` pour aligner selon l'origine du `Transform`.
- Applique les decalages locaux (`offset`), la hierarchie des parents et la rotation locale.
- Met a l'echelle cumulee (`CalculateCumulativeScale`) et ajuste l'opacite via `Text::opacity`.
- Trie les entites par `z_index` avant dessin pour garantir l'ordre d'affichage.

## Signature principale

```cpp
void DrawTextRenderSystem(
    Eng::registry &reg,
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts);
```

## Notes

- `InitializeText` fixe la taille du texte a `characterSize * 10` puis applique l'echelle cumulee divisee par 10 pour garder une coherence avec les sprites.
- En cas d'echec de chargement de police, une erreur est loggee dans `std::cerr` mais `is_loaded` passe a `true` pour eviter des relances infinies.
- Le champ `offset` permet de decaler le texte localement par rapport a la position du `Transform` sans modifier la hierarchie.
