#include "include/components/GameplayComponents.hpp"

#include <string>
#include <utility>

namespace Rtype::Client::Component {

// ------------------------------ TimedEvents ------------------------------

TimedEvents::TimedEvents(std::function<void(int)> action, float cooldown_max) {
    AddCooldownAction(std::move(action), cooldown_max);
}

void TimedEvents::AddCooldownAction(
    std::function<void(int)> action, float cooldown_max) {
    CooldownAction cd_action;
    cd_action.action = std::move(action);
    cd_action.cooldown_max = cooldown_max;
    cd_action.cooldown = 0.0f;
    cooldown_actions.push_back(std::move(cd_action));
}

// ------------------------------ EnemyShootTag -----------------------------

EnemyShootTag::EnemyShootTag(float speed, int damage, sf::Vector2f offset)
    : speed_projectile(speed),
      damage_projectile(damage),
      offset_shoot_position(offset) {}

void EnemyShootTag::AddFrameEvent(const std::string &animation_name, int frame,
    std::function<void(int)> action) {
    frame_events.emplace_back(animation_name, frame, std::move(action));
}

EnemyShootTag::FrameEvent::FrameEvent(
    const std::string &anim_name, int frame, std::function<void(int)> act)
    : animation_name(anim_name),
      trigger_frame(frame),
      action(std::move(act)),
      triggered(false) {}

}  // namespace Rtype::Client::Component
