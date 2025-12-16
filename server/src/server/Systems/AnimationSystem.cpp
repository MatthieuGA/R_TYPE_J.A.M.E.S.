#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

/**
 * @brief Advance the animation to the next frame and update the
 * drawable rectangle.
 *
 * @param animation Animation state to advance
 * @param drawable Drawable component to update
 */
void NextFrame(Component::AnimatedSprite::Animation &animation) {
    animation.current_frame++;
    if (animation.current_frame >= animation.totalFrames) {
        if (animation.loop)
            animation.current_frame = 0;
        else
            animation.current_frame = animation.totalFrames - 1;
    }
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
void AnimationSystem(Engine::registry &reg,
    Engine::sparse_array<Component::AnimatedSprite> &anim_sprites) {
    for (auto &&[i, anim_sprite] : make_indexed_zipper(anim_sprites)) {
        // Get the current animation using the helper method
        auto *animation = anim_sprite.GetCurrentAnimation();
        if (animation == nullptr)
            continue;

        if (!anim_sprite.animated)
            continue;

        anim_sprite.elapsedTime += TICK_RATE_SECONDS;
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
                } else {
                    NextFrame(*animation);
                }
            }
        }
    }
}
}  // namespace server
