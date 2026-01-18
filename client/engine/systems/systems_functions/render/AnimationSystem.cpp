#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Apply animation texture path to drawable metadata.
 *
 * Backend will load/cache the texture when drawing.
 */
bool ApplyAnimationTexture(
    Com::AnimatedSprite::Animation &animation, Com::Drawable &drawable) {
    if (animation.texture_path.empty()) {
        return true;
    }
    drawable.texture_path = animation.texture_path;
    animation.is_loaded = true;
    return true;
}

/**
 * @brief Update the current rectangle according to the current frame.
 *
 * Computes the texture rect for the current frame. Supports both:
 * - Horizontal strip: frames laid out in a single row
 * - Grid layout: frames wrap across multiple rows based on sheet width
 *
 * If first_frame_position is (0,0), assumes grid layout.
 * Otherwise, uses first_frame_position as the starting point for a strip.
 *
 * @param animation Animation containing frame metadata and sheet_width
 * @param drawable Drawable to update with calculated rectangle
 * @param game_world GameWorld for querying texture dimensions (if needed)
 */
void SetFrame(Com::AnimatedSprite::Animation &animation,
    Com::Drawable &drawable, GameWorld &game_world) {
    if (animation.frameWidth <= 0 || animation.frameHeight <= 0)
        return;

    int left, top;

    // Grid layout detection: if first_frame is at origin, use grid math
    if (animation.first_frame_position.x == 0.0f &&
        animation.first_frame_position.y == 0.0f) {
        // Grid layout: determine sheet width
        int sheet_width = animation.sheet_width;

        // Auto-detect sheet width from texture if not set
        if (sheet_width <= 0 && game_world.GetRenderContext()) {
            // Use animation texture path, or fallback to drawable's texture
            const char *query_path = !animation.texture_path.empty()
                                         ? animation.texture_path.c_str()
                                         : drawable.texture_path.c_str();

            if (query_path && query_path[0] != '\0') {
                auto tex_size =
                    game_world.GetRenderContext()->GetTextureSize(query_path);
                sheet_width = static_cast<int>(tex_size.x);
            }
        }

        if (sheet_width <= 0) {
            // Fallback: can't determine sheet width, use horizontal strip
            left = animation.current_frame * animation.frameWidth;
            top = 0;
        } else {
            // Prevent divide by zero if frameWidth is somehow 0 or negative
            if (animation.frameWidth <= 0) {
                return;
            }
            const int frames_per_row = sheet_width / animation.frameWidth;
            const int row = animation.current_frame / frames_per_row;
            const int col = animation.current_frame % frames_per_row;
            left = col * animation.frameWidth;
            top = row * animation.frameHeight;
        }
    } else {
        // Horizontal strip layout: offset from first_frame_position
        left = static_cast<int>(animation.first_frame_position.x) +
               animation.current_frame * animation.frameWidth;
        top = static_cast<int>(animation.first_frame_position.y);
    }

    Engine::Graphics::IntRect rect(
        left, top, animation.frameWidth, animation.frameHeight);
    drawable.current_rect = rect;
}

/**
 * @brief Advance the animation to the next frame and update the
 * drawable rectangle.
 *
 * @param animation Animation state to advance
 * @param drawable Drawable component to update
 * @param game_world GameWorld for querying texture dimensions
 */
void NextFrame(Com::AnimatedSprite::Animation &animation,
    Com::Drawable &drawable, GameWorld &game_world) {
    animation.current_frame++;
    if (animation.current_frame >= animation.totalFrames) {
        if (animation.loop)
            animation.current_frame = 0;
        else
            animation.current_frame = animation.totalFrames - 1;
    }
    SetFrame(animation, drawable, game_world);
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
 * @param game_world GameWorld containing RenderContext for texture queries
 * @param dt Delta time (seconds) since last update
 * @param anim_sprites Sparse array of AnimatedSprite components
 * @param drawables Sparse array of Drawable components
 */
void AnimationSystem(Eng::registry &reg, GameWorld &game_world, const float dt,
    Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
    Eng::sparse_array<Com::Drawable> &drawables) {
    for (auto &&[i, anim_sprite, drawable] :
        make_indexed_zipper(anim_sprites, drawables)) {
        auto *animation = anim_sprite.GetCurrentAnimation();
        if (animation == nullptr)
            continue;

        ApplyAnimationTexture(*animation, drawable);
        SetFrame(*animation, drawable, game_world);

        if (!drawable.is_loaded || !anim_sprite.animated)
            continue;

        anim_sprite.elapsedTime += dt;
        if (animation->frameDuration > 0.0f) {
            while (anim_sprite.elapsedTime >= animation->frameDuration) {
                anim_sprite.elapsedTime -= animation->frameDuration;
                if (!animation->loop &&
                    animation->current_frame == animation->totalFrames - 1) {
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
                    auto *defaultAnim = anim_sprite.GetCurrentAnimation();
                    if (defaultAnim != nullptr) {
                        ApplyAnimationTexture(*defaultAnim, drawable);
                        SetFrame(*defaultAnim, drawable, game_world);
                    }
                } else {
                    NextFrame(*animation, drawable, game_world);
                }
            }
        }
    }
}
}  // namespace Rtype::Client
