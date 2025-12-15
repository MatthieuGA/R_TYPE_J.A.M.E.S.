#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Load the texture for an animation if not already loaded.
 *
 * Uses the rendering engine to load textures in a backend-agnostic way.
 *
 * @param animation Animation to load
 * @param game_world Game world containing rendering engine
 * @return true if loading succeeded or was already loaded, false otherwise
 */
bool LoadAnimation(
    Com::AnimatedSprite::Animation &animation, GameWorld &game_world) {
    if (animation.isLoaded)
        return true;

    if (animation.path.empty() || !game_world.rendering_engine_)
        return false;

    // Load texture via rendering engine
    bool loaded = game_world.rendering_engine_->LoadTexture(
        animation.texture_id, animation.path);

    if (!loaded) {
        std::cerr << "ERROR: Failed to load animation texture: "
                  << animation.path << " (ID: " << animation.texture_id << ")"
                  << std::endl;
        return false;
    }

    animation.isLoaded = true;
    return true;
}

/**
 * @brief System to load all animations in AnimatedSprite components.
 *
 * This system iterates through all AnimatedSprite components and loads
 * any animations that haven't been loaded yet via the rendering engine.
 * This is useful when adding new animations at runtime.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world containing rendering engine
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void LoadAnimationSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, anim_sprite] : make_indexed_zipper(animated_sprites)) {
        for (auto &[name, animation] : anim_sprite.animations) {
            if (!animation.isLoaded) {
                LoadAnimation(animation, game_world);
            }
        }
    }
}

}  // namespace Rtype::Client
