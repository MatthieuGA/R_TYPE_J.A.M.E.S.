#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief System to handle entity removal after death animation completes.
 *
 * This system checks entities marked with AnimationDeath component and
 * monitors their death animation. Once the animation finishes playing,
 * the entity is removed from the registry, cleaning up all its components.
 *
 * The death animation must be non-looping. When it finishes, AnimationSystem
 * automatically switches it back to "Default". We detect this and remove
 * the entity.
 *
 * @param reg ECS registry used to access entities and components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param animation_deaths Sparse array of AnimationDeath components.
 */
void DeathAnimationSystem(Eng::registry &reg,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::AnimationDeath> &animation_deaths) {
    for (auto &&[i, animation_death] : make_indexed_zipper(animation_deaths)) {
        Engine::entity entity = reg.EntityFromIndex(i);

        // Check if entity has animated sprite and death animation
        if (animated_sprites.has(i)) {
            auto &anim_sprite = animated_sprites[i];

            std::cout << "[DeathAnimSystem] Entity " << i
                      << " current anim: " << anim_sprite->currentAnimation
                      << ", animated: " << anim_sprite->animated << std::endl;

            if (anim_sprite->currentAnimation == "Default") {
                // Remove the entity entirely
                std::cout << "[DeathAnimSystem] Killing entity " << i
                          << std::endl;
                reg.KillEntity(entity);
            }
        } else {
            // If no animated sprite, remove immediately
            std::cout << "[DeathAnimSystem] Killing entity " << i
                      << " (no anim sprite)" << std::endl;
            reg.KillEntity(entity);
        }
    }
}

}  // namespace Rtype::Client
