#include <iostream>
#include <string>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Sets the texture rectangle on the drawable according to the
 * current frame stored in the AnimatedSprite.
 *
 * This directly updates the drawable's texture_rect (IntRect) based on
 * the animation frame and layout. The actual drawing happens in
 * DrawableSystem.
 *
 * @param anim_sprite Animated sprite state (current_frame, frame_width/height)
 * @param drawable Drawable component to update texture_rect on
 * @param game_world Game world for accessing video backend
 */
void SetFrame(Com::AnimatedSprite &anim_sprite, Com::Drawable &drawable,
    GameWorld &game_world) {
    if (!game_world.video_backend_ || anim_sprite.frame_width == 0) {
        return;
    }

    // Get texture size from backend
    Engine::Graphics::Vector2f texture_size =
        game_world.video_backend_->GetTextureSize(drawable.texture_id);

    if (texture_size.x == 0) {
        return;
    }

    const int columns =
        static_cast<int>(texture_size.x) / anim_sprite.frame_width;
    if (columns == 0) {
        return;
    }

    const int left =
        static_cast<int>(anim_sprite.first_frame_position.x) +
        (anim_sprite.current_frame % columns) * anim_sprite.frame_width;
    const int top =
        static_cast<int>(anim_sprite.first_frame_position.y) +
        (anim_sprite.current_frame / columns) * anim_sprite.frame_height;

    // Update drawable's texture_rect (will be used by DrawableSystem)
    drawable.texture_rect = Engine::Graphics::IntRect(
        left, top, anim_sprite.frame_width, anim_sprite.frame_height);
}

/**
 * @brief Advance the animated sprite to the next frame and update the
 * drawable rectangle.
 *
 * @param anim_sprite Animated sprite state to advance
 * @param drawable Drawable component to update
 * @param game_world Game world for backend access
 */
void NextFrame(Com::AnimatedSprite &anim_sprite, Com::Drawable &drawable,
    GameWorld &game_world) {
    anim_sprite.current_frame++;
    if (anim_sprite.current_frame >= anim_sprite.total_frames) {
        if (anim_sprite.loop) {
            anim_sprite.current_frame = 0;
        } else {
            anim_sprite.current_frame = anim_sprite.total_frames - 1;
        }
    }
    SetFrame(anim_sprite, drawable, game_world);
}

/**
 * @brief System that updates all animated sprites each frame.
 *
 * Accumulates elapsed time and advances frames according to each
 * AnimatedSprite::frame_duration. If an animation is not looping it will
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
    static int frame_count = 0;
    bool debug = (frame_count++ % 60 == 0);
    for (auto &&[i, anim_sprite, drawable] :
        make_indexed_zipper(anim_sprites, drawables)) {
        if (debug &&
            drawable.texture_id.find("r-typesheet2") != std::string::npos) {
            std::cout << "[AnimationSystem] Entity " << i
                      << ": animated=" << anim_sprite.animated
                      << ", loaded=" << drawable.is_loaded
                      << ", frame=" << anim_sprite.current_frame << std::endl;
        }
        if (!drawable.is_loaded || !anim_sprite.animated) {
            SetFrame(anim_sprite, drawable, game_world);
            continue;
        }

        anim_sprite.elapsed_time += dt;
        if (anim_sprite.frame_duration > 0.0f) {
            while (anim_sprite.elapsed_time >= anim_sprite.frame_duration) {
                anim_sprite.elapsed_time -= anim_sprite.frame_duration;
                NextFrame(anim_sprite, drawable, game_world);
                if (!anim_sprite.loop &&
                    anim_sprite.current_frame == anim_sprite.total_frames - 1)
                    break;
            }
        }
    }
}
}  // namespace Rtype::Client
