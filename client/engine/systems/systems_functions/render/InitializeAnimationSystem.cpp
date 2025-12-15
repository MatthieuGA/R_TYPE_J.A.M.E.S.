#include <iostream>

#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Initialize a drawable that uses an animated sprite sheet.
 *
 * Loads the texture via video backend and sets the texture_rect to the
 * current frame.
 *
 * @param drawable Drawable component to initialize
 * @param animated_sprite AnimatedSprite providing frame info
 * @param game_world Game world containing video backend
 */
void InitializeDrawableAnimated(Com::Drawable &drawable,
    const Com::AnimatedSprite &animated_sprite, GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        std::cerr << "ERROR: video_backend is null!" << std::endl;
        return;
    }

    // Load texture via video backend
    bool loaded = game_world.rendering_engine_->LoadTexture(
        drawable.texture_id, drawable.sprite_path);

    if (!loaded) {
        std::cerr << "ERROR: Failed to load animated sprite: "
                  << drawable.sprite_path << " (ID: " << drawable.texture_id
                  << ")" << std::endl;
        return;
    }

    // Set initial texture rect to first frame
    // Get the current animation from the animations map
    auto it =
        animated_sprite.animations.find(animated_sprite.currentAnimation);
    if (it == animated_sprite.animations.end()) {
        // Fallback to "Default" if current animation not found
        it = animated_sprite.animations.find("Default");
        if (it == animated_sprite.animations.end() ||
            animated_sprite.animations.empty()) {
            std::cerr << "ERROR: No animations found for AnimatedSprite"
                      << std::endl;
            return;
        }
        // Use the first available animation as fallback
        it = animated_sprite.animations.begin();
    }

    const auto &anim = it->second;
    drawable.texture_rect = Engine::Graphics::IntRect(
        static_cast<int>(anim.first_frame_position.x) +
            (anim.current_frame * anim.frameWidth),
        static_cast<int>(anim.first_frame_position.y), anim.frameWidth,
        anim.frameHeight);

    drawable.is_loaded = true;
}

/**
 * @brief System to initialize drawables for entities that have an
 * AnimatedSprite component.
 *
 * Ensures textures are loaded and the sprite rectangle is set for
 * animated sprites.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world containing video backend
 * @param transforms Sparse array of Transform components
 * @param drawables Sparse array of Drawable components
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    GameWorld &game_world, Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    for (auto &&[i, transform, drawable, animated_sprite] :
        make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.is_loaded) {
            InitializeDrawableAnimated(drawable, animated_sprite, game_world);
        }
    }
}
}  // namespace Rtype::Client
