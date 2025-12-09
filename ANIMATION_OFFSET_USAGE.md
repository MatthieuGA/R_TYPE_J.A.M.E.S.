# Animation Offset Feature

## Description

Le système d'offset d'animation permet de décaler les sprites animés par rapport à leur position de transformation. Ceci est particulièrement utile lorsque vous avez des animations de tailles différentes pour une même entité et que vous souhaitez maintenir un point d'ancrage visuel cohérent.

## Usage

### Ajout d'une animation avec offset

Lors de l'ajout d'une nouvelle animation à un composant `AnimatedSprite`, vous pouvez spécifier un offset :

```cpp
AnimatedSprite anim_sprite(64, 64, 0.1f, true);

// Ajouter une animation avec un offset de (10, -5) pixels
anim_sprite.AddAnimation(
    "jump",                          // nom de l'animation
    "player_jump.png",               // chemin du sprite
    64,                              // largeur de frame
    80,                              // hauteur de frame  
    8,                               // nombre total de frames
    0.1f,                            // durée par frame
    false,                           // loop
    sf::Vector2f(0.0f, 0.0f),       // position de la première frame
    sf::Vector2f(10.0f, -5.0f)      // offset à appliquer
);
```

### Paramètres de l'offset

- **Type**: `sf::Vector2f`
- **Par défaut**: `(0.0f, 0.0f)` (pas d'offset)
- **X positif**: Décale vers la droite
- **X négatif**: Décale vers la gauche
- **Y positif**: Décale vers le bas
- **Y négatif**: Décale vers le haut

## Cas d'usage

### Exemple 1: Animations de tailles différentes

Si vous avez une animation de marche en 32x32 et une animation de saut en 32x48, vous pouvez utiliser un offset pour aligner les pieds du personnage :

```cpp
// Animation de marche (32x32) - pas d'offset nécessaire
anim_sprite.AddAnimation("walk", "walk.png", 32, 32, 4, 0.1f);

// Animation de saut (32x48) - décaler vers le haut de 16 pixels
anim_sprite.AddAnimation("jump", "jump.png", 32, 48, 6, 0.1f, 
    false, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, -16.0f));
```

### Exemple 2: Ajustement visuel fin

Pour affiner la position visuelle d'une animation spécifique sans modifier la position de l'entité :

```cpp
// Décaler légèrement une animation d'attaque vers l'avant
anim_sprite.AddAnimation("attack", "attack.png", 64, 64, 8, 0.08f,
    false, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(15.0f, 0.0f));
```

## Implémentation technique

L'offset est appliqué automatiquement lors du rendu par le `DrawableSystem`. Le système :

1. Récupère l'animation active via `GetCurrentAnimation()`
2. Ajoute l'offset de l'animation à la position calculée du monde
3. Applique la position finale au sprite avant le rendu

L'offset est indépendant de la transformation (position, rotation, scale) et s'applique après le calcul de la position hiérarchique.

## Structure Animation

Le champ `offset` a été ajouté à la structure `Animation` :

```cpp
struct Animation {
    // ... autres champs ...
    sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f);
    // ...
};
```

## Note importante

L'offset est spécifique à chaque animation. Ainsi, changer d'animation peut changer l'offset appliqué. Assurez-vous que vos offsets sont cohérents avec votre design visuel.
