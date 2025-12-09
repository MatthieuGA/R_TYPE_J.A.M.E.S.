# Game State System

**Fichier source:** `client/Engine/Systems/SystemsFunctions/GameStateSystem.cpp`

**But:** Gérer les transitions de scène en appelant les hooks `DestroyScene` et `InitScene` quand le prochain état change.

**Composants utilisés:**
- `SceneManagement` (current, next, scenes: `std::unordered_map<std::string, std::shared_ptr<Scene_A>>`)

## Comportement

- Ignore la passe si `next` est vide ou identique à `current`.
- Si la scène courante existe dans `scenes`, appelle `DestroyScene(reg)` pour nettoyer l'état sortant.
- Si la scène cible existe dans `scenes`, appelle `InitScene(reg)` pour initialiser le nouvel état.
- Met à jour `current` avec `next`, puis réinitialise `next` à une chaîne vide pour éviter les rechargements involontaires.

## Signature principale

```cpp
void GameStateSystem(Eng::registry &reg,
    Eng::sparse_array<Component::SceneManagement> &sceneManagements);
```

## Notes
- `Scene_A` définit les hooks `InitScene` et `DestroyScene` invoqués par ce système.
- Le champ `scenes` doit être pré-rempli avec les scènes disponibles et leurs noms-clés.
- `next` doit être positionné par d'autres logiques (UI, réseau, scripts) avant d'exécuter ce système afin que le changement soit effectif lors de la passe.
