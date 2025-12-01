#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
void setDrawableOrigin(Com::Drawable &drawable, const Com::Transform &transform) {
    sf::Vector2f origin = get_offset_from_transform(transform,
        sf::Vector2f(
            static_cast<float>(drawable.texture.getSize().x),
            static_cast<float>(drawable.texture.getSize().y)));
    drawable.sprite.setOrigin(origin);
}

void initializeDrawable(Com::Drawable &drawable, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    setDrawableOrigin(drawable, transform);
    drawable.isLoaded = true;
}

void setDrawableAnimationOrigin(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    sf::Vector2f origin = get_offset_from_animated_transform(transform,
        animatedSprite);
    drawable.sprite.setOrigin(origin);
}

void initializeDrawableAnimated(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    setDrawableAnimationOrigin(drawable, animatedSprite, transform);
    drawable.isLoaded = true;
}

void drawableSystem(Eng::registry &reg, sf::RenderWindow &window,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::AnimatedSprite> const &animatedSprites) {
    // Draw all entities with Transform and Drawable and AnimatedSprite components
    for (auto &&[i, tranform, drawable, animatedSprite] :
    make_indexed_zipper(transforms, drawables, animatedSprites)) {
        if (!drawable.isLoaded)
            initializeDrawableAnimated(drawable, animatedSprite, tranform);
    }

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, tranform, drawable] :
    make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            initializeDrawable(drawable, tranform);

        drawable.sprite.setPosition(sf::Vector2f(tranform.x, tranform.y));
        drawable.sprite.setScale(sf::Vector2f(tranform.scale, tranform.scale));
        drawable.sprite.setRotation(tranform.rotationDegrees);
        window.draw(drawable.sprite);
    }
}
}  // namespace Rtype::Client
