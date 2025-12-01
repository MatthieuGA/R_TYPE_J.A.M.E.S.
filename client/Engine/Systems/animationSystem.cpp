#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void animationSystem(Eng::registry &reg, const sf::Clock &deltaTimeClock,
Eng::sparse_array<Com::AnimatedSprite> &animatedSprites,
Eng::sparse_array<Com::Drawable> &drawables) {
    for (auto &&[i, animatedSprite, drawable] :
    make_indexed_zipper(animatedSprites, drawables)) {
        if (!drawable.isLoaded)
            continue;
        animatedSprite.elapsedTime += deltaTimeClock.getElapsedTime().asSeconds();
        if (animatedSprite.elapsedTime >= animatedSprite.frameDuration) {
            animatedSprite.currentFrame++;
            if (animatedSprite.currentFrame >= animatedSprite.totalFrames) {
                if (animatedSprite.loop)
                    animatedSprite.currentFrame = 0;
                else
                    animatedSprite.currentFrame = animatedSprite.totalFrames - 1;
            }
            int columns = drawable.texture.getSize().x / animatedSprite.frameWidth;
            int left = (animatedSprite.currentFrame % columns) * animatedSprite.frameWidth;
            int top = (animatedSprite.currentFrame / columns) * animatedSprite.frameHeight;
            drawable.sprite.setTextureRect(sf::IntRect(left, top,
                animatedSprite.frameWidth, animatedSprite.frameHeight));
            animatedSprite.elapsedTime = 0.0f;
        }
    }
}
}  // namespace Rtype::Client
