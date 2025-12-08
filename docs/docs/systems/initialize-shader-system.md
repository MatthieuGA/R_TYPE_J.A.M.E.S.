# Initialize Shader System

**Fichier source:** `client/engine/systems/SystemsFunctions/initializeShaderSystem.cpp`

**But:** Charger et initialiser les shaders (fragment shaders) pour les composants `Shader`.

**Composants utilisés:**
- `Shader` (contient `shaderPath`, `uniforms_float`, `isLoaded`, `shader` shared_ptr)

## Comportement

- Pour chaque composant `Shader` non chargé et avec un `shaderPath`, charge un `sf::Shader` en mémoire.
- Initialise l'uniform `texture` à `sf::Shader::CurrentTexture` et applique les uniforms float fournis.
- En cas d'échec, affiche un message d'erreur et remet le pointeur `shader` à `nullptr`.

## Signature principale

```cpp
void InitializeShaderSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Shader> &shaders);
```

## Notes
- Les shaders sont des `std::shared_ptr<sf::Shader>`.
- Le système est idempotent : il n'essaiera pas de recharger un shader déjà marqué `isLoaded`.
