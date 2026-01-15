#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Load the texture for an animation if not already loaded.
 *
 * Marks animation as loaded. Backend will load texture via path.
 *
 * @param animation Animation to load
 * @return true if path is valid and marked loaded, false otherwise
 */
bool LoadAnimation(Com::AnimatedSprite::Animation &animation) {
    if (animation.is_loaded)
        return true;

    if (animation.texture_path.empty())
        return false;

    // Backend will load texture via path; just mark as loaded here
    animation.is_loaded = true;
    return true;
}

/**
 * @brief System to load all animations in AnimatedSprite components.
 *
 * This system iterates through all AnimatedSprite components and loads
 * any animations that haven't been loaded yet. This is useful when adding
 * new animations at runtime.
 *
 * @param reg Engine registry (unused)
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void LoadAnimationSystem(Eng::registry &reg,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, anim_sprite] : make_indexed_zipper(animated_sprites)) {
        for (auto &[name, animation] : anim_sprite.animations) {
            if (!animation.is_loaded) {
                LoadAnimation(animation);
            }
        }
    }
}
}  // namespace Rtype::Client
