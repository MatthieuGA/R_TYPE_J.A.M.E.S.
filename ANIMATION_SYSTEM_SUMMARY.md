# Système d'Animation Amélioré - Résumé

## Nouvelles Fonctionnalités

### 1. Méthodes utilitaires dans `AnimatedSprite`

#### `AddAnimation()`
Ajoute facilement une nouvelle animation à la map :
```cpp
animated_sprite.AddAnimation("run", "player_run.png", 64, 64, 8, 0.1f, true);
```

#### `SetCurrentAnimation()`
Change l'animation en cours avec option de reset :
```cpp
// Change et réinitialise l'animation
animated_sprite.SetCurrentAnimation("attack", true);

// Change sans réinitialiser (continue à la frame actuelle)
animated_sprite.SetCurrentAnimation("idle", false);
```

#### `GetCurrentAnimation()`
Obtient un pointeur vers l'animation courante :
```cpp
auto* anim = animated_sprite.GetCurrentAnimation();
if (anim != nullptr) {
    std::cout << "Frame: " << anim->current_frame << std::endl;
}
```

### 2. Système `LoadAnimationSystem`

Nouveau système qui charge automatiquement toutes les animations non-chargées :
- Parcourt tous les `AnimatedSprite`
- Charge les textures des animations avec `isLoaded == false`
- Gère les erreurs de chargement

**Fichier:** `client/engine/systems/systems_functions/render/LoadAnimationSystem.cpp`

### 3. Simplification du `AnimationSystem`

Le système d'animation utilise maintenant `GetCurrentAnimation()` pour plus de clarté :
```cpp
auto* animation = anim_sprite.GetCurrentAnimation();
if (animation != nullptr) {
    // Traiter l'animation...
}
```

## Exemple d'utilisation complète

```cpp
// 1. Créer une entité avec AnimatedSprite
auto entity = registry.spawn_entity();
auto& anim_sprite = registry.add_component<Component::AnimatedSprite>(
    entity, 64, 64, 0.1f, true);

// 2. Ajouter plusieurs animations
anim_sprite.AddAnimation("idle", "hero_idle.png", 64, 64, 4, 0.15f, true);
anim_sprite.AddAnimation("run", "hero_run.png", 64, 64, 8, 0.1f, true);
anim_sprite.AddAnimation("jump", "hero_jump.png", 64, 64, 6, 0.12f, false);

// 3. Définir l'animation de départ
anim_sprite.SetCurrentAnimation("idle");

// 4. Le système LoadAnimationSystem chargera automatiquement les textures

// 5. Changer d'animation selon la logique du jeu
if (player_is_moving) {
    anim_sprite.SetCurrentAnimation("run");
} else {
    anim_sprite.SetCurrentAnimation("idle");
}

// 6. Vérifier si une animation non-looping est terminée
auto* anim = anim_sprite.GetCurrentAnimation();
if (anim && !anim->loop && anim->current_frame == anim->totalFrames - 1) {
    // L'animation est terminée
    anim_sprite.SetCurrentAnimation("idle");
}
```

## Fichiers modifiés/créés

### Modifiés
- `client/include/components/RenderComponent.hpp` - Ajout des méthodes utilitaires
- `client/engine/systems/InitRegistrySystems.hpp` - Déclaration de LoadAnimationSystem
- `client/engine/systems/systems_functions/render/AnimationSystem.cpp` - Utilisation de GetCurrentAnimation()

### Créés
- `client/engine/systems/systems_functions/render/LoadAnimationSystem.cpp` - Nouveau système
- `docs/docs/systems/animation-system-guide.md` - Guide complet
- `client/engine/systems/systems_functions/ExampleAnimationUsage.cpp` - Exemples pratiques

## Avantages

✅ **Simplicité** - Ajouter une animation en une seule ligne
✅ **Sécurité** - Vérification automatique de l'existence des animations
✅ **Flexibilité** - Possibilité de reset ou continuer une animation
✅ **Automatisation** - Chargement automatique des textures
✅ **Clarté** - Code plus lisible avec les méthodes helper
✅ **Performance** - Chargement lazy des textures (uniquement quand nécessaire)

## Notes importantes

- L'animation "Default" sert toujours de fallback
- Les animations sont stockées dans une `std::map<std::string, Animation>`
- Le chargement est automatique via `LoadAnimationSystem`
- `SetCurrentAnimation()` retourne `false` si l'animation n'existe pas
- Par défaut, changer d'animation réinitialise à la frame 0
