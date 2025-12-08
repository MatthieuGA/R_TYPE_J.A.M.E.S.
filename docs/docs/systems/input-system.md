# Input System

**Fichier source:** `client/engine/systems/SystemsFunctions/InputSystem.cpp`

**But:** Lire l'état des touches du clavier et remplir le composant `Inputs`.

**Composants utilisés:**
- `Inputs` (horizontal, vertical, shoot)

## Comportement

- Réinitialise `horizontal`, `vertical` et `shoot` à 0/false au début de la passe.
- Lit les touches suivantes (mappage local) :
  - `Q` = gauche (-1.0f)
  - `D` = droite (+1.0f)
  - `Z` = haut (-1.0f)
  - `S` = bas (+1.0f)
  - `Space` = tir (`shoot = true`)

## Signature principale

```cpp
void InputSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs);
```

## Notes
- Le système utilise `sf::Keyboard::isKeyPressed` et fonctionne côté client.
- Remapper les touches doit se faire dans le code si nécessaire.
