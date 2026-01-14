#include <vector>

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
    // Collect entities to kill to avoid modifying sparse arrays during
    // iteration
    std::vector<Engine::entity> entities_to_kill;

    for (auto &&[i, animation_death] : make_indexed_zipper(animation_deaths)) {
        Engine::entity entity = reg.EntityFromIndex(i);

        // Check if entity has animated sprite and death animation
        if (animated_sprites.has(i)) {
            auto &anim_sprite = animated_sprites[i];

            if (anim_sprite->currentAnimation == "Default") {
                // Mark entity for removal
                entities_to_kill.push_back(entity);
            }
        } else {
            // If no animated sprite, mark for immediate removal
            entities_to_kill.push_back(entity);
        }
    }

    // Now kill entities after iteration is complete
    for (Engine::entity entity : entities_to_kill) {
        reg.KillEntity(entity);
    }
}

}  // namespace Rtype::Client
