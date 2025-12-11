# Animation System - Usage Guide

This guide explains how to use the animation system to manage multiple animations per entity.

## Add an animation

To add a new animation to an `AnimatedSprite`, call `AddAnimation()`:

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

// Add a running animation
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

// Add an attack animation
animated_sprite->AddAnimation(
    "attack",
    "player_attack.png",
    64, 64, 5, 0.08f, false  // Ne boucle pas
);

// Add an idle animation
animated_sprite->AddAnimation(
    "idle",
    "player_idle.png",
    64, 64, 4, 0.15f, true
);
```

## Load animations

`LoadAnimationSystem` automatically loads any animations that are not yet loaded.

Add it to your system registry:

```cpp
// Dans InitRegistrySystems.cpp
void InitRegistrySystems(GameWorld &game_world) {
    // ... other systems ...

    // Add the animation loading system
    game_world.registry.add_system<Com::AnimatedSprite>(LoadAnimationSystem);
    
    // Add the animation system (already present)
    game_world.registry.add_system<Com::AnimatedSprite, Com::Drawable>(
        [](Eng::registry &reg, Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
           Eng::sparse_array<Com::Drawable> &drawables) {
            AnimationSystem(reg, 0.016f, anim_sprites, drawables);
        });
}
```

## Change the current animation

To change the active animation, use `SetCurrentAnimation()`:

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

// Switch to "run" and reset it (default behavior)
if (animated_sprite->SetCurrentAnimation("run")) {
    // L'animation a été changée avec succès
}

// Switch to "attack" without resetting (keep current frame)
animated_sprite->SetCurrentAnimation("attack", false);

// Example inside a gameplay system
void PlayerMovementSystem(...) {
    for (auto &&[i, player, velocity, animated_sprite] : 
         make_indexed_zipper(players, velocities, animated_sprites)) {
        
        if (velocity.vx != 0 || velocity.vy != 0) {
            // Player is moving -> run animation
            animated_sprite.SetCurrentAnimation("run");
        } else {
            // Player is idle -> idle animation
            animated_sprite.SetCurrentAnimation("idle");
        }
    }
}
```

## Get the current animation

Use `GetCurrentAnimation()` to inspect animation properties:

```cpp
auto& animated_sprite = registry.get_components<Component::AnimatedSprite>()[entity_id];

auto* current_anim = animated_sprite->GetCurrentAnimation();
if (current_anim != nullptr) {
    std::cout << "Current frame: " << current_anim->current_frame
              << "/" << current_anim->totalFrames << std::endl;

    // Check if a non-looping animation has finished
    if (!current_anim->loop &&
        current_anim->current_frame == current_anim->totalFrames - 1) {
        std::cout << "Animation finished!" << std::endl;
    }
}
```

## Full example: Combat system

```cpp
void PlayerCombatSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> &players,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    
    for (auto &&[i, player, input, anim_sprite] : 
         make_indexed_zipper(players, inputs, animated_sprites)) {
        
        // Check if player starts an attack
        if (input.attack_pressed && !player.is_attacking) {
            player.is_attacking = true;
            anim_sprite.SetCurrentAnimation("attack", true);
        }
        
        // Check if the attack animation is finished
        if (player.is_attacking) {
            auto* anim = anim_sprite.GetCurrentAnimation();
            if (anim != nullptr && anim->current_frame == anim->totalFrames - 1) {
                player.is_attacking = false;
                anim_sprite.SetCurrentAnimation("idle");
            }
        }
    }
}
```

## Recommended execution order

1. **LoadAnimationSystem** – load new animations
2. **Gameplay systems** – switch current animation based on logic
3. **AnimationSystem** – advance animation frames
4. **DrawableSystem** – render the sprite

## Important notes

- Animations are auto-loaded the first time they are used.
- `SetCurrentAnimation()` returns `false` if the animation name does not exist.
- By default, switching animation resets the frame to 0.
- The "Default" animation acts as a fallback if the requested animation is missing.
