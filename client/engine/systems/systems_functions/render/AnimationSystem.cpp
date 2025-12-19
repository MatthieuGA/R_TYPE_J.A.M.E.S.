#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

inline sf::IntRect ToSFML(const Engine::Graphics::IntRect &r) {
    return sf::IntRect(r.left, r.top, r.width, r.height);
}

inline Engine::Graphics::IntRect ToEngine(const sf::IntRect &r) {
    return Engine::Graphics::IntRect(r.left, r.top, r.width, r.height);
}

/**
 * @brief Load and apply the texture from the animation to the drawable.
 *
 * If the animation has its own texture loaded, it will be applied to the
 * drawable. Otherwise, the drawable's original texture is used.
 *
 * @param animation Animation containing the texture
 * @param drawable Drawable component to update
 * @return true if the animation texture was loaded and applied, false
 * otherwise
 */
bool ApplyAnimationTexture(
    Com::AnimatedSprite::Animation &animation, Com::Drawable &drawable) {
    // If animation has no path, use the drawable's original texture
    if (animation.path.empty()) {
        drawable.sprite.setTexture(drawable.texture, true);
        return true;
    }

    // If animation has a path but isn't loaded, load it
    if (!animation.isLoaded) {
        if (!animation.texture.loadFromFile(animation.path)) {
            std::cerr << "ERROR: Failed to load animation texture from "
                      << animation.path << "\n";
            return false;
        }
        animation.sprite.setTexture(animation.texture, true);
        animation.isLoaded = true;
    }

    // Apply the animation's texture to the drawable if it's loaded
    if (animation.isLoaded) {
        drawable.sprite.setTexture(animation.texture, true);
        return true;
    }

    return false;
}

/**
 * @brief Sets the texture rectangle on the drawable according to the
 * current frame stored in the Animation.
 *
 * @param animation Animation state (current_frame, frameWidth/Height)
 * @param drawable Drawable component containing the texture and sprite
 */
void SetFrame(
    Com::AnimatedSprite::Animation &animation, Com::Drawable &drawable) {
    // Determine which texture to use for calculating columns
    sf::Vector2u textureSize;
    if (animation.isLoaded) {
        textureSize = animation.texture.getSize();
    } else {
        textureSize = drawable.texture.getSize();
    }

    if (textureSize.x == 0 || animation.frameWidth == 0)
        return;
    const int columns = textureSize.x / animation.frameWidth;
    if (columns == 0)
        return;
    const int left =
        animation.first_frame_position.x +
        (animation.current_frame % columns) * animation.frameWidth;
    const int top =
        animation.first_frame_position.y +
        (animation.current_frame / columns) * animation.frameHeight;
    Engine::Graphics::IntRect rect(
        left, top, animation.frameWidth, animation.frameHeight);
    drawable.sprite.setTextureRect(ToSFML(rect));
}

/**
 * @brief Advance the animation to the next frame and update the
 * drawable rectangle.
 *
 * @param animation Animation state to advance
 * @param drawable Drawable component to update
 */
void NextFrame(
    Com::AnimatedSprite::Animation &animation, Com::Drawable &drawable) {
    animation.current_frame++;
    if (animation.current_frame >= animation.totalFrames) {
        if (animation.loop)
            animation.current_frame = 0;
        else
            animation.current_frame = animation.totalFrames - 1;
    }
    SetFrame(animation, drawable);
}

/**
 * @brief System that updates all animated sprites each frame.
 *
 * Accumulates elapsed time and advances frames according to each
 * Animation::frameDuration. If an animation is not looping it will
 * remain on the last frame. Uses the currentAnimation name to select
 * the active animation from the animations map.
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
        // Get the current animation using the helper method
        auto *animation = anim_sprite.GetCurrentAnimation();
        if (animation == nullptr)
            continue;

        // Apply the animation's texture to the drawable
        ApplyAnimationTexture(*animation, drawable);

        // Always set the frame to ensure correct texture rect
        SetFrame(*animation, drawable);

        if (!drawable.isLoaded || !anim_sprite.animated)
            continue;

        anim_sprite.elapsedTime += dt;
        if (animation->frameDuration > 0.0f) {
            while (anim_sprite.elapsedTime >= animation->frameDuration) {
                anim_sprite.elapsedTime -= animation->frameDuration;
                if (!animation->loop &&
                    animation->current_frame == animation->totalFrames - 1) {
                    // Animation finished, switch back to default
                    if (anim_sprite.animationQueue.empty()) {
                        anim_sprite.SetCurrentAnimation("Default", true);
                    } else {
                        auto [nextAnim, frame] =
                            anim_sprite.animationQueue.back();
                        anim_sprite.animationQueue.erase(
                            anim_sprite.animationQueue.end() - 1);
                        anim_sprite.SetCurrentAnimation(
                            nextAnim, false, false);
                        auto *newAnim = anim_sprite.GetCurrentAnimation();
                        if (newAnim != nullptr)
                            newAnim->current_frame = frame;
                    }
                    // Re-apply the default animation texture
                    auto *defaultAnim = anim_sprite.GetCurrentAnimation();
                    if (defaultAnim != nullptr) {
                        ApplyAnimationTexture(*defaultAnim, drawable);
                        SetFrame(*defaultAnim, drawable);
                    }
                } else {
                    NextFrame(*animation, drawable);
                }
            }
        }
    }
}
}  // namespace Rtype::Client
