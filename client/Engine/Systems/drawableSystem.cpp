#include <iostream>

#include "Engine/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
void SetDrawableOrigin(
    Com::Drawable &drawable, const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromTransform(transform,
        sf::Vector2f(static_cast<float>(drawable.texture.getSize().x),
            static_cast<float>(drawable.texture.getSize().y)));
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawable(
    Com::Drawable &drawable, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
                  << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableOrigin(drawable, transform);
    drawable.isLoaded = true;
}

void SetDrawableAnimationOrigin(Com::Drawable &drawable,
    const Com::AnimatedSprite &animatedSprite,
    const Com::Transform &transform) {
    sf::Vector2f origin =
        GetOffsetFromAnimatedTransform(transform, animatedSprite);
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawableAnimated(Com::Drawable &drawable,
    const Com::AnimatedSprite &animatedSprite,
    const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
                  << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableAnimationOrigin(drawable, animatedSprite, transform);
    drawable.isLoaded = true;
}

void DrawableSystem(Eng::registry &reg, sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    // Draw all entities with Transform / Drawable / AnimatedSprite components
    for (auto &&[i, tranform, drawable, animated_sprite] :
        make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.isLoaded)
            InitializeDrawableAnimated(drawable, animated_sprite, tranform);
    }

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, tranform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, tranform);

        drawable.sprite.setPosition(sf::Vector2f(tranform.x, tranform.y));
        drawable.sprite.setScale(sf::Vector2f(tranform.scale, tranform.scale));
        drawable.sprite.setRotation(tranform.rotationDegrees);
        window.draw(drawable.sprite);
    }
}
}  // namespace Rtype::Client
