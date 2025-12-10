#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void HandleFrameBaseEvents(Eng::registry &reg, int entityId,
    Com::EnemyShootTag &enemy_shoot, Com::Transform const &transform,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites, int i) {
    auto &anim_sprite = animated_sprites[i];
    auto *current_anim = anim_sprite->GetCurrentAnimation();

    if (current_anim == nullptr)
        return;
    int current_frame = current_anim->currentFrame;
    std::string current_anim_name = anim_sprite->currentAnimation;

    // Check all frame events
    for (auto &frame_event : enemy_shoot.frame_events) {
        // Check if this event matches current animation and frame
        if (frame_event.animation_name == current_anim_name &&
            frame_event.trigger_frame == current_frame &&
            !frame_event.triggered) {
            // Execute the action callback
            if (frame_event.action)
                frame_event.action(entityId);
            frame_event.triggered = true;
        } else if (current_frame == 0) {
            // Reset triggered flag when animation resets
            frame_event.triggered = false;
        }
    }
}

void ShootEnemySystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::EnemyShootTag> &enemy_shoot_tags,
    Eng::sparse_array<Com::EnemyTag> const &enemy_tags) {
    for (auto &&[i, transform, enemy_shoot, enemy_tag] :
        make_indexed_zipper(transforms, enemy_shoot_tags, enemy_tags)) {
        // Handle frame-based events if configured
        if (!enemy_shoot.frame_events.empty() && animated_sprites.has(i)) {
            HandleFrameBaseEvents(
                reg, i, enemy_shoot, transform, animated_sprites, i);
        }
    }
}
}  // namespace Rtype::Client
