#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

/**
 * @brief Handle a single frame event for an entity.
 *
 * @param reg The registry containing components.
 * @param entityId The ID of the entity.
 * @param frame_event The frame event to handle.
 * @param anim_sprite The animated sprite component of the entity.
 */
void HandleOneFrameEvent(Eng::registry &reg, int entityId,
    Com::FrameEvents::FrameEvent &frame_event,
    Com::AnimatedSprite &anim_sprite) {
    Com::AnimatedSprite::Animation *current_anim =
        anim_sprite.GetCurrentAnimation();
    if (current_anim == nullptr)
        return;

    // Check if this event matches current animation and frame
    if (frame_event.animation_name == anim_sprite.currentAnimation &&
        frame_event.trigger_frame == current_anim->current_frame &&
        !frame_event.triggered) {
        // Execute the action callback
        if (frame_event.action)
            frame_event.action(entityId);
        frame_event.triggered = true;
    } else if (current_anim->current_frame == 0) {
        // Reset triggered flag when animation resets
        frame_event.triggered = false;
    }
}

/**
 * @brief Handle all frame-based events for an entity.
 *
 * @param reg The registry containing components.
 * @param i The ID of the entity.
 * @param frame_events The frame events component of the entity.
 * @param transform The transform component of the entity.
 * @param anim_sprite The animated sprite component of the entity.
 */
void HandleFrameBaseEvents(Eng::registry &reg, int i,
    Com::FrameEvents &frame_events, Com::Transform const &transform,
    Com::AnimatedSprite &anim_sprite) {
    auto *current_anim = anim_sprite.GetCurrentAnimation();

    if (current_anim == nullptr)
        return;
    int current_frame = current_anim->current_frame;
    std::string current_anim_name = anim_sprite.currentAnimation;

    // Check all frame events
    for (auto &frame_event : frame_events.frame_events) {
        HandleOneFrameEvent(reg, i, frame_event, anim_sprite);
    }
}

/**
 * @brief System to process frame-based events for entities.
 *
 * @param reg The registry containing components.
 * @param game_world The game world context.
 * @param transforms The sparse array of Transform components.
 * @param animated_sprites The sparse array of AnimatedSprite components.
 * @param frame_events The sparse array of FrameEvents components.
 */
void FrameBaseEventSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::FrameEvents> &frame_events) {
    for (auto &&[i, transform, enemy_shoot, animated_sprite] :
        make_indexed_zipper(transforms, frame_events, animated_sprites)) {
        // Handle frame-based events if configured
        if (!enemy_shoot.frame_events.empty()) {
            HandleFrameBaseEvents(
                reg, i, enemy_shoot, transform, animated_sprite);
        }
    }
}
}  // namespace Rtype::Client
