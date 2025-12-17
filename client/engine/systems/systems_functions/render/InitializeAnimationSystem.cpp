#include <iostream>

#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

inline sf::Vector2f ToSFML(const Engine::Graphics::Vector2f &v) {
    return sf::Vector2f(v.x, v.y);
}

inline Engine::Graphics::Vector2f ToEngine(const sf::Vector2f &v) {
    return Engine::Graphics::Vector2f(v.x, v.y);
}

/**
 * @brief Compute and apply the origin for an animated drawable.
 *
 * Uses `GetOffsetFromAnimatedTransform` to calculate the correct origin
 * based on the animated frame size and transform.
 *
 * @param drawable Drawable component to update
 * @param animatedSprite AnimatedSprite containing frame dimensions
 * @param transform Transform used for offset calculation
 */
void SetDrawableAnimationOrigin(Com::Drawable &drawable,
    const Com::AnimatedSprite &animatedSprite,
    const Com::Transform &transform) {
    auto it = animatedSprite.animations.find(animatedSprite.currentAnimation);
    if (it == animatedSprite.animations.end())
        return;

    sf::Vector2f origin =
        ToSFML(GetOffsetFromAnimatedTransform(transform, animatedSprite));
    drawable.sprite.setOrigin(-origin);
}

/**
 * @brief Initialize a drawable that uses an animated sprite sheet.
 *
 * Loads the texture, sets the origin for animated frames and initializes the
 * sprite IntRect to the current frame.
 *
 * @param drawable Drawable component to initialize
 * @param animatedSprite AnimatedSprite providing frame info
 * @param transform Transform for origin calculation
 */
void InitializeDrawableAnimated(Com::Drawable &drawable,
    const Com::AnimatedSprite &animatedSprite,
    const Com::Transform &transform) {
    auto it = animatedSprite.animations.find(animatedSprite.currentAnimation);
    if (it == animatedSprite.animations.end())
        return;

    const auto &animation = it->second;

    if (!drawable.texture.loadFromFile(drawable.spritePath)) {
        std::cerr << "ERROR: Failed to load sprite from "
                  << drawable.spritePath << "\n";
    } else {
        drawable.sprite.setTexture(drawable.texture, true);
    }
    SetDrawableAnimationOrigin(drawable, animatedSprite, transform);
    drawable.sprite.setTextureRect(
        sf::IntRect(animation.current_frame * animation.frameWidth, 0,
            animation.frameWidth, animation.frameHeight));
    drawable.isLoaded = true;
}

/**
 * @brief System to initialize drawables for entities that have an
 * AnimatedSprite component.
 *
 * Ensures textures are loaded and the sprite rectangle/origin are set for
 * animated sprites.
 *
 * @param reg Engine registry (unused)
 * @param transforms Sparse array of Transform components
 * @param drawables Sparse array of Drawable components
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    // Else draw entities with Transform and Drawable components
    for (auto &&[i, transform, drawable, animated_sprite] :
        make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.isLoaded)
            InitializeDrawableAnimated(drawable, animated_sprite, transform);
    }
}
}  // namespace Rtype::Client
