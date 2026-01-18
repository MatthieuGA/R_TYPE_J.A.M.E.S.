#include "server/GameplayComponents.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/Vector2f.hpp"

namespace server::Component {

// ------------------------------ TimedEvents ------------------------------

TimedEvents::TimedEvents(std::function<void(int)> action, float cooldown_max) {
    AddCooldownAction(std::move(action), cooldown_max);
}

void TimedEvents::AddCooldownAction(
    std::function<void(int)> action, float cooldown_max, float delay) {
    CooldownAction cd_action;
    cd_action.action = std::move(action);
    cd_action.cooldown_max = cooldown_max;
    cd_action.cooldown = delay;
    cooldown_actions.push_back(std::move(cd_action));
}

// ------------------------------ FrameEvents ------------------------------

void FrameEvents::AddFrameEvent(const std::string &animation_name, int frame,
    std::function<void(int)> action) {
    frame_events.emplace_back(animation_name, frame, std::move(action));
}

FrameEvents::FrameEvent::FrameEvent(
    const std::string &anim_name, int frame, std::function<void(int)> act)
    : animation_name(anim_name),
      trigger_frame(frame),
      action(std::move(act)),
      triggered(false) {}

FrameEvents::FrameEvents(const std::string &animation_name, int frame,
    std::function<void(int)> action) {
    frame_events.emplace_back(animation_name, frame, std::move(action));
}

FrameEvents::FrameEvents(std::vector<FrameEvent> events)
    : frame_events(std::move(events)) {}

// ------------------------------ EnemyShootTag -----------------------------

EnemyShootTag::EnemyShootTag(
    float speed, int damage, vector2f offset, int score)
    : speed_projectile(speed),
      damage_projectile(damage),
      offset_shoot_position(offset),
      score_value(score) {}

// ------------------------------ AnimatedSprite -----------------------------

// AnimatedSprite constructors
AnimatedSprite::AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    : currentAnimation("Default"), animated(true), elapsedTime(0.0f) {
    Animation defaultAnimation(totalFrames, frameDuration, loop);
    animations["Default"] = defaultAnimation;
}

AnimatedSprite::AnimatedSprite(int current_frame)
    : currentAnimation("Default"), animated(false), elapsedTime(0.0f) {
    Animation defaultAnimation(1, 0.0f, false);
    defaultAnimation.current_frame = current_frame;
    animations["Default"] = defaultAnimation;
}

// AnimatedSprite methods
void AnimatedSprite::AddAnimation(
    const std::string &name, int totalFrames, float frameDuration, bool loop) {
    animations[name] = Animation(totalFrames, frameDuration, loop);
}

bool AnimatedSprite::SetCurrentAnimation(
    const std::string &name, bool reset, bool push_to_queue) {
    auto it = animations.find(name);
    if (it == animations.end())
        return false;

    const bool should_queue = push_to_queue && currentAnimation != "Default" &&
                              name != "Default" && name != currentAnimation &&
                              name != "Death";
    if (should_queue) {
        animationQueue.push_back(
            {currentAnimation, animations[currentAnimation].current_frame});
    }
    if (name == "Death")
        animationQueue.clear();
    currentAnimation = name;
    if (reset) {
        it->second.current_frame = 0;
        elapsedTime = 0.0f;
    }
    return true;
}

AnimatedSprite::Animation *AnimatedSprite::GetCurrentAnimation() {
    auto it = animations.find(currentAnimation);
    if (it == animations.end()) {
        it = animations.find("Default");
        if (it == animations.end())
            return nullptr;
    }
    return &(it->second);
}

const AnimatedSprite::Animation *AnimatedSprite::GetCurrentAnimation() const {
    auto it = animations.find(currentAnimation);
    if (it == animations.end()) {
        it = animations.find("Default");
        if (it == animations.end())
            return nullptr;
    }
    return &(it->second);
}

std::vector<std::string> AnimatedSprite::GetAnimationNames() const {
    std::vector<std::string> names;
    for (const auto &[name, animation] : animations) {
        names.push_back(name);
    }
    return names;
}

}  // namespace server::Component
