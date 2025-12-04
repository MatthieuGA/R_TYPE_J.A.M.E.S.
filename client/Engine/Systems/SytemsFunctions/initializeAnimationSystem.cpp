#include <iostream>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {

void SetDrawableAnimationOrigin(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromAnimatedTransform(transform,
        animatedSprite);
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawableAnimated(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath)) {
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    } else {
        drawable.sprite.setTexture(drawable.texture, true);
    }
    SetDrawableAnimationOrigin(drawable, animatedSprite, transform);
    drawable.sprite.setTextureRect(sf::IntRect(animatedSprite.currentFrame *
        animatedSprite.frameWidth, 0, animatedSprite.frameWidth,
        animatedSprite.frameHeight));
    drawable.isLoaded = true;
}

void InitializeDrawableAnimatedSystem(Eng::registry &reg,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    // Else draw entities with Transform and Drawable components
    for (auto &&[i, tranform, drawable, animated_sprite] :
    make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.isLoaded)
            InitializeDrawableAnimated(drawable, animated_sprite, tranform);
    }
}
}  // namespace Rtype::Client
