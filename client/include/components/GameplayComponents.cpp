#include "include/components/GameplayComponents.hpp"

#include <string>
#include <utility>

namespace Rtype::Client::Component {

EnemyShootTag::EnemyShootTag(
    float cooldown, float speed, int damage, sf::Vector2f offset)
    : shoot_cooldown_max(cooldown),
      speed_projectile(speed),
      damage_projectile(damage),
      offset_shoot_position(offset),
      cooldown_action(nullptr) {}

void EnemyShootTag::AddFrameEvent(const std::string &animation_name, int frame,
    std::function<void(int)> action) {
    frame_events.emplace_back(animation_name, frame, std::move(action));
}

void EnemyShootTag::SetCooldownAction(std::function<void(int)> action) {
    cooldown_action = std::move(action);
}

EnemyShootTag::FrameEvent::FrameEvent(
    const std::string &anim_name, int frame, std::function<void(int)> act)
    : animation_name(anim_name),
      trigger_frame(frame),
      action(std::move(act)),
      triggered(false) {}

}  // namespace Rtype::Client::Component
