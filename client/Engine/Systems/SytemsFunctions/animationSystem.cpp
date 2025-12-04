#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void NextFrame(Com::AnimatedSprite &anim_sprite, Com::Drawable &drawable) {
    anim_sprite.currentFrame++;
    if (anim_sprite.currentFrame >= anim_sprite.totalFrames) {
        if (anim_sprite.loop)
            anim_sprite.currentFrame = 0;
        else
            anim_sprite.currentFrame = anim_sprite.totalFrames - 1;
    }
    if (drawable.texture.getSize().x == 0 || anim_sprite.frameWidth == 0)
        return;
    const int columns = drawable.texture.getSize().x / anim_sprite.frameWidth;
    if (columns == 0)
        return;
    const int left =
        (anim_sprite.currentFrame % columns) * anim_sprite.frameWidth;
    const int top =
        (anim_sprite.currentFrame / columns) * anim_sprite.frameHeight;
    drawable.sprite.setTextureRect(sf::IntRect(
        left, top, anim_sprite.frameWidth, anim_sprite.frameHeight));
    anim_sprite.elapsedTime = 0.0f;
}

void AnimationSystem(Eng::registry &reg, const float dt,
Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
Eng::sparse_array<Com::Drawable> &drawables) {
    for (auto &&[i, anim_sprite, drawable] :
    make_indexed_zipper(anim_sprites, drawables)) {
        if (!drawable.isLoaded) continue;
        anim_sprite.elapsedTime += dt;

        if (anim_sprite.elapsedTime >= anim_sprite.frameDuration)
            NextFrame(anim_sprite, drawable);
    }
}
}  // namespace Rtype::Client
