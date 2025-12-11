#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Load the texture for an animation if not already loaded.
 *
 * @param animation Animation to load
 * @return true if loading succeeded or was already loaded, false otherwise
 */
bool LoadAnimation(Com::AnimatedSprite::Animation &animation) {
    if (animation.isLoaded)
        return true;

    if (animation.path.empty())
        return false;

    if (!animation.texture.loadFromFile(animation.path)) {
        std::cerr << "ERROR: Failed to load animation texture from "
                  << animation.path << "\n";
        return false;
    }

    animation.sprite.setTexture(animation.texture, true);
    animation.isLoaded = true;
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
            if (!animation.isLoaded) {
                LoadAnimation(animation);
            }
        }
    }
}
}  // namespace Rtype::Client
