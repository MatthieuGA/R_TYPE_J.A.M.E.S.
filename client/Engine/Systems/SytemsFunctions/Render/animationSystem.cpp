#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Sets the texture rectangle on the drawable according to the
 * current frame stored in the AnimatedSprite.
 *
 * @param anim_sprite Animated sprite state (currentFrame, frameWidth/Height)
 * @param drawable Drawable component containing the texture and sprite
 */
void SetFrame(Com::AnimatedSprite &anim_sprite, Com::Drawable &drawable) {
    if (drawable.texture.getSize().x == 0 || anim_sprite.frameWidth == 0)
        return;
    const int columns = drawable.texture.getSize().x / anim_sprite.frameWidth;
    if (columns == 0) return;
    const int left = anim_sprite.first_frame_position.x +
        (anim_sprite.currentFrame % columns) * anim_sprite.frameWidth;
    const int top = anim_sprite.first_frame_position.y +
        (anim_sprite.currentFrame / columns) * anim_sprite.frameHeight;
    drawable.sprite.setTextureRect(sf::IntRect(
        left, top, anim_sprite.frameWidth, anim_sprite.frameHeight));
}

/**
 * @brief Advance the animated sprite to the next frame and update the
 * drawable rectangle.
 *
 * @param anim_sprite Animated sprite state to advance
 * @param drawable Drawable component to update
 */
void NextFrame(Com::AnimatedSprite &anim_sprite, Com::Drawable &drawable) {
    anim_sprite.currentFrame++;
    if (anim_sprite.currentFrame >= anim_sprite.totalFrames) {
        if (anim_sprite.loop)
            anim_sprite.currentFrame = 0;
        else
            anim_sprite.currentFrame = anim_sprite.totalFrames - 1;
    }
    SetFrame(anim_sprite, drawable);
}

/**
 * @brief System that updates all animated sprites each frame.
 *
 * Accumulates elapsed time and advances frames according to each
 * AnimatedSprite::frameDuration. If an animation is not looping it will
 * remain on the last frame.
 *
 * @param reg Engine registry (unused in current implementation)
 * @param dt Delta time (seconds) since last update
 * @param anim_sprites Sparse array of AnimatedSprite components
 * @param drawables Sparse array of Drawable components
 */
void AnimationSystem(Eng::registry &reg, const float dt,
Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
Eng::sparse_array<Com::Drawable> &drawables) {
    for (auto &&[i, anim_sprite, drawable] :
        make_indexed_zipper(anim_sprites, drawables)) {
        if (!drawable.isLoaded || !anim_sprite.animated) {
            SetFrame(anim_sprite, drawable);
            continue;
        }

        anim_sprite.elapsedTime += dt;
        if (anim_sprite.frameDuration > 0.0f) {
            while (anim_sprite.elapsedTime >= anim_sprite.frameDuration) {
                anim_sprite.elapsedTime -= anim_sprite.frameDuration;
                NextFrame(anim_sprite, drawable);
                if (!anim_sprite.loop &&
                    anim_sprite.currentFrame == anim_sprite.totalFrames - 1)
                    break;
            }
        }
    }
}
}  // namespace Rtype::Client
