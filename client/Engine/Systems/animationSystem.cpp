#include "Engine/initRegistrySystems.hpp"

namespace Rtype::Client {
void NextFrame(Com::AnimatedSprite &animated_sprite, Com::Drawable &drawable) {
    animated_sprite.currentFrame++;
    if (animated_sprite.currentFrame >= animated_sprite.totalFrames) {
        if (animated_sprite.loop)
            animated_sprite.currentFrame = 0;
        else
            animated_sprite.currentFrame = animated_sprite.totalFrames - 1;
    }
    const int columns = drawable.texture.getSize().x / animated_sprite.frameWidth;
    const int left = (animated_sprite.currentFrame % columns) * animated_sprite.frameWidth;
    const int top = (animated_sprite.currentFrame / columns) * animated_sprite.frameHeight;
    drawable.sprite.setTextureRect(sf::IntRect(left, top,
        animated_sprite.frameWidth, animated_sprite.frameHeight));
    animated_sprite.elapsedTime = 0.0f;
}

void AnimationSystem(Eng::registry &reg, const sf::Clock &delta_time_clock,
Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
Eng::sparse_array<Com::Drawable> &drawables) {
    for (auto &&[i, animated_sprite, drawable] :
    make_indexed_zipper(animated_sprites, drawables)) {
        if (!drawable.isLoaded) continue;
        animated_sprite.elapsedTime += delta_time_clock.getElapsedTime().asSeconds();

        if (animated_sprite.elapsedTime >= animated_sprite.frameDuration)
            NextFrame(animated_sprite, drawable);
    }
}
}  // namespace Rtype::Client
