#include <iostream>
#include <string>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Load and apply an animation's texture if not already loaded.
 *
 * @param animation Animation to load texture for
 * @param drawable Drawable component to update
 * @param game_world Game world for backend access
 * @return true if texture is loaded, false otherwise
 */
bool ApplyAnimationTexture(Com::AnimatedSprite::Animation &animation,
    Com::Drawable &drawable, GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        return false;
    }

    // If animation has no path, use the drawable's original texture
    if (animation.path.empty()) {
        // Fallback to drawable's sprite_path (for Default animation)
        if (!drawable.sprite_path.empty() && !drawable.texture_id.empty()) {
            return drawable.is_loaded;
        }
        return false;
    }

    // If animation isn't loaded yet, load it now
    if (!animation.isLoaded) {
        std::string texture_id = animation.texture_id.empty()
                                     ? animation.path
                                     : animation.texture_id;

        bool loaded = game_world.rendering_engine_->LoadTexture(
            texture_id, animation.path);

        if (!loaded) {
            std::cerr << "[AnimationSystem] ERROR: Failed to load animation "
                         "texture: "
                      << animation.path << " (ID: " << texture_id << ")"
                      << std::endl;
            return false;
        }

        animation.texture_id = texture_id;
        animation.isLoaded = true;
        std::cout << "[AnimationSystem] Loaded animation texture: "
                  << texture_id << std::endl;
    }

    // Update drawable to use this animation's texture
    if (animation.isLoaded) {
        drawable.texture_id = animation.texture_id;
        return true;
    }

    return false;
}

/**
 * @brief Sets the texture rectangle on the drawable according to the
 * current frame stored in the Animation.
 *
 * @param animation Animation state (current_frame, frameWidth/Height)
 * @param drawable Drawable component to update texture_rect
 * @param game_world Game world for accessing rendering engine
 */
void SetFrame(Com::AnimatedSprite::Animation &animation,
    Com::Drawable &drawable, GameWorld &game_world) {
    if (!game_world.rendering_engine_ || animation.frameWidth == 0) {
        return;
    }

    // Use animation's texture_id if it has one, otherwise fall back to
    // drawable's texture_id
    std::string texture_id = !animation.texture_id.empty()
                                 ? animation.texture_id
                                 : drawable.texture_id;

    // Get texture size from rendering engine
    Engine::Graphics::Vector2f texture_size =
        game_world.rendering_engine_->GetTextureSize(texture_id);

    if (texture_size.x == 0) {
        return;
    }

    const int columns =
        static_cast<int>(texture_size.x) / animation.frameWidth;
    if (columns == 0) {
        return;
    }

    const int left =
        static_cast<int>(animation.first_frame_position.x) +
        (animation.current_frame % columns) * animation.frameWidth;
    const int top =
        static_cast<int>(animation.first_frame_position.y) +
        (animation.current_frame / columns) * animation.frameHeight;

    // Update drawable's texture_rect (will be used by DrawableSystem)
    drawable.texture_rect = Engine::Graphics::IntRect(
        left, top, animation.frameWidth, animation.frameHeight);
}

/**
 * @brief Advance the animation to the next frame and update the
 * drawable rectangle.
 *
 * @param animation Animation state to advance
 * @param drawable Drawable component to update
 * @param game_world Game world for backend access
 */
void NextFrame(Com::AnimatedSprite::Animation &animation,
    Com::Drawable &drawable, GameWorld &game_world) {
    animation.current_frame++;
    if (animation.current_frame >= animation.totalFrames) {
        if (animation.loop) {
            animation.current_frame = 0;
        } else {
            animation.current_frame = animation.totalFrames - 1;
        }
    }
    SetFrame(animation, drawable, game_world);
}

/**
 * @brief System that updates all animated sprites each frame.
 *
 * Accumulates elapsed time and advances frames according to each
 * animation's frameDuration. If an animation is not looping it will
 * remain on the last frame.
 *
 * @param reg Engine registry (unused in current implementation)
 * @param game_world Game world for backend access
 * @param dt Delta time (seconds) since last update
 * @param anim_sprites Sparse array of AnimatedSprite components
 * @param drawables Sparse array of Drawable components
 */
void AnimationSystem(Eng::registry &reg, GameWorld &game_world, const float dt,
    Eng::sparse_array<Com::AnimatedSprite> &anim_sprites,
    Eng::sparse_array<Com::Drawable> &drawables) {
    static int debug_counter = 0;
    bool debug = (debug_counter++ % 60 == 0);

    for (auto &&[i, anim_sprite, drawable] :
        make_indexed_zipper(anim_sprites, drawables)) {
        // Get current animation
        auto *animation = anim_sprite.GetCurrentAnimation();
        if (animation == nullptr) {
            continue;
        }

        // Debug charging animation (opacity changes, 8 frames)
        static int frame_counter = 0;
        bool is_charging =
            (animation->totalFrames == 8 && animation->frameWidth == 33);
        if (is_charging &&
            (frame_counter++ % 60 == 0 || drawable.opacity > 0.0f)) {
            std::cout << "[AnimationSystem] Charging entity " << i
                      << ": animated=" << anim_sprite.animated
                      << ", is_loaded=" << drawable.is_loaded
                      << ", opacity=" << drawable.opacity
                      << ", current_frame=" << animation->current_frame
                      << ", path='" << animation->path << "'"
                      << ", texture_id='" << animation->texture_id << "'"
                      << std::endl;
        }

        // Load animation texture if not loaded yet (handles both animated and
        // static)
        if (!ApplyAnimationTexture(*animation, drawable, game_world)) {
            continue;
        }

        // Always set the frame to ensure correct texture rect (even for static
        // sprites)
        SetFrame(*animation, drawable, game_world);

        // Only advance frames if drawable is loaded AND animated flag is true
        if (!drawable.is_loaded || !anim_sprite.animated) {
            continue;
        }

        // Update animation timing
        anim_sprite.elapsedTime += dt;
        if (animation->frameDuration > 0.0f) {
            while (anim_sprite.elapsedTime >= animation->frameDuration) {
                anim_sprite.elapsedTime -= animation->frameDuration;

                // Check if animation finished before advancing
                if (!animation->loop &&
                    animation->current_frame == animation->totalFrames - 1) {
                    // Animation finished, switch back to default or next
                    // queued animation
                    std::cout << "[AnimationSystem] Animation '"
                              << anim_sprite.currentAnimation
                              << "' finished for entity " << i
                              << ", switching to Default" << std::endl;

                    if (anim_sprite.animationQueue.empty()) {
                        anim_sprite.SetCurrentAnimation("Default", true);
                    } else {
                        // Handle animation queue
                        auto [nextAnim, frame] =
                            anim_sprite.animationQueue.back();
                        anim_sprite.animationQueue.erase(
                            anim_sprite.animationQueue.end() - 1);
                        anim_sprite.SetCurrentAnimation(
                            nextAnim, false, false);
                        auto *newAnim = anim_sprite.GetCurrentAnimation();
                        if (newAnim != nullptr) {
                            newAnim->current_frame = frame;
                        }
                    }

                    // Re-apply the new animation texture and frame
                    auto *newAnimation = anim_sprite.GetCurrentAnimation();
                    if (newAnimation != nullptr && newAnimation->isLoaded) {
                        drawable.texture_id = newAnimation->texture_id;
                        SetFrame(*newAnimation, drawable, game_world);
                    }
                    break;
                } else {
                    NextFrame(*animation, drawable, game_world);
                }
            }
        }
    }
}

}  // namespace Rtype::Client
