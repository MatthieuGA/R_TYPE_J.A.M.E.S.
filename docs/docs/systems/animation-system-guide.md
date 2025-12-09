# Animation System - Guide d'utilisation

Ce guide explique comment utiliser le nouveau système d'animations pour gérer facilement plusieurs animations par entité.

## Ajouter une animation

Pour ajouter une nouvelle animation à un `AnimatedSprite`, utilisez la méthode `AddAnimation()` :

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

// Ajouter une animation de course
animated_sprite->AddAnimation(
    "run",                          // Nom de l'animation
    "player_run.png",               // Chemin de la texture (relatif à assets/images/)
    64,                             // Largeur d'une frame
    64,                             // Hauteur d'une frame
    8,                              // Nombre total de frames
    0.1f,                           // Durée de chaque frame (secondes)
    true,                           // Boucle l'animation
    sf::Vector2f(0.0f, 0.0f)       // Position de la première frame (optionnel)
);

// Ajouter une animation d'attaque
animated_sprite->AddAnimation(
    "attack",
    "player_attack.png",
    64, 64, 5, 0.08f, false  // Ne boucle pas
);

// Ajouter une animation idle
animated_sprite->AddAnimation(
    "idle",
    "player_idle.png",
    64, 64, 4, 0.15f, true
);
```

## Charger les animations

Le système `LoadAnimationSystem` charge automatiquement toutes les animations qui n'ont pas encore été chargées.

Ajoutez-le à votre registre de systèmes :

```cpp
// Dans InitRegistrySystems.cpp
void InitRegistrySystems(GameWorld &game_world) {
    // ... autres systèmes ...
    
    // Ajouter le système de chargement d'animations
    game_world.registry.add_system<Com::AnimatedSprite>(LoadAnimationSystem);
    
    // Ajouter le système d'animation (déjà existant)
    game_world.registry.add_system<Com::AnimatedSprite, Com::Drawable>(
        [](Eng::registry &reg, Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
           Eng::sparse_array<Com::Drawable> &drawables) {
            AnimationSystem(reg, 0.016f, anim_sprites, drawables);
        });
}
```

## Changer l'animation en cours

Pour changer l'animation jouée, utilisez `SetCurrentAnimation()` :

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

// Changer vers l'animation "run" et la réinitialiser (par défaut)
if (animated_sprite->SetCurrentAnimation("run")) {
    // L'animation a été changée avec succès
}

// Changer vers "attack" sans réinitialiser (continuer où elle était)
animated_sprite->SetCurrentAnimation("attack", false);

// Exemple dans un système de gameplay
void PlayerMovementSystem(...) {
    for (auto &&[i, player, velocity, animated_sprite] : 
         make_indexed_zipper(players, velocities, animated_sprites)) {
        
        if (velocity.vx != 0 || velocity.vy != 0) {
            // Le joueur bouge -> animation de course
            animated_sprite.SetCurrentAnimation("run");
        } else {
            // Le joueur est immobile -> animation idle
            animated_sprite.SetCurrentAnimation("idle");
        }
    }
}
```

## Obtenir l'animation courante

Utilisez `GetCurrentAnimation()` pour accéder aux propriétés de l'animation :

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

auto* current_anim = animated_sprite->GetCurrentAnimation();
if (current_anim != nullptr) {
    std::cout << "Frame actuelle: " << current_anim->currentFrame 
              << "/" << current_anim->totalFrames << std::endl;
    
    // Vérifier si l'animation est terminée (pour les animations non-looping)
    if (!current_anim->loop && 
        current_anim->currentFrame == current_anim->totalFrames - 1) {
        std::cout << "Animation terminée!" << std::endl;
    }
}
```

## Exemple complet : Système de combat

```cpp
void PlayerCombatSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> &players,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    
    for (auto &&[i, player, input, anim_sprite] : 
         make_indexed_zipper(players, inputs, animated_sprites)) {
        
        // Vérifier si le joueur attaque
        if (input.attack_pressed && !player.is_attacking) {
            player.is_attacking = true;
            anim_sprite.SetCurrentAnimation("attack", true);
        }
        
        // Vérifier si l'animation d'attaque est terminée
        if (player.is_attacking) {
            auto* anim = anim_sprite.GetCurrentAnimation();
            if (anim != nullptr && anim->currentFrame == anim->totalFrames - 1) {
                player.is_attacking = false;
                anim_sprite.SetCurrentAnimation("idle");
            }
        }
    }
}
```

## Ordre d'exécution recommandé

1. **LoadAnimationSystem** - Charge les nouvelles animations
2. **Systèmes de gameplay** - Changent l'animation courante selon la logique
3. **AnimationSystem** - Met à jour les frames de l'animation
4. **DrawableSystem** - Affiche le sprite

## Notes importantes

- Les animations sont chargées automatiquement la première fois qu'elles sont utilisées
- `SetCurrentAnimation()` retourne `false` si l'animation n'existe pas
- Par défaut, changer d'animation réinitialise la frame à 0
- L'animation "Default" sert de fallback si l'animation courante n'existe pas
