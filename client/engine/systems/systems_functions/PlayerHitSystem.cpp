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
void PlayerHitSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Component::PlayerTag> &player_tags) {
    for (auto &&[i, player_tag] : make_indexed_zipper(player_tags)) {
        Engine::entity entity = reg.EntityFromIndex(i);

        // Check if entity has animated sprite
        if (animated_sprites.has(i)) {
            auto &anim_sprite = animated_sprites[i];

            // If current animation is "Hit" and on last frame, switch back
            if (anim_sprite->currentAnimation == "Hit") {
                anim_sprite->elapsedTime += game_world.last_delta_;
                if (anim_sprite->elapsedTime >=
                    anim_sprite->GetCurrentAnimation()->frameDuration) {
                    anim_sprite->SetCurrentAnimation("Default", true);
                    auto *currentAnim = anim_sprite->GetCurrentAnimation();
                    
                    // Update drawable texture_id to match the Default animation
                    if (currentAnim != nullptr && currentAnim->isLoaded) {
                        auto &drawables = reg.GetComponents<Component::Drawable>();
                        if (drawables.has(i)) {
                            drawables[i]->texture_id = currentAnim->texture_id;
                        }
                    }
                    
                    anim_sprite->elapsedTime = 0.0f;
                }
            }
        }
    }
}
}  // namespace Rtype::Client
