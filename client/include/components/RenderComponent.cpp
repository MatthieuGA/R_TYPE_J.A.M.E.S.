#include "include/components/RenderComponent.hpp"

#include <string>
#include <utility>

namespace Rtype::Client::Component {

// AnimatedSprite constructors
AnimatedSprite::AnimatedSprite(int frameWidth, int frameHeight,
    float frameDuration, bool loop,
    Engine::Graphics::Vector2f first_frame_position, int totalFrames)
    : currentAnimation("Default"), animated(true), elapsedTime(0.0f) {
    Animation defaultAnimation("", frameWidth, frameHeight, totalFrames,
        frameDuration, loop, first_frame_position);
    animations["Default"] = defaultAnimation;
}

AnimatedSprite::AnimatedSprite(
    int frameWidth, int frameHeight, int current_frame)
    : currentAnimation("Default"), animated(false), elapsedTime(0.0f) {
    Animation defaultAnimation("", frameWidth, frameHeight, 1, 0.0f, false);
    defaultAnimation.current_frame = current_frame;
    animations["Default"] = defaultAnimation;
}

// AnimatedSprite methods
void AnimatedSprite::AddAnimation(const std::string &name,
    const std::string &path, int frameWidth, int frameHeight, int totalFrames,
    float frameDuration, bool loop,
    Engine::Graphics::Vector2f first_frame_position,
    Engine::Graphics::Vector2f offset) {
    animations[name] = Animation(path, frameWidth, frameHeight, totalFrames,
        frameDuration, loop, first_frame_position, offset);
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

// Text move constructor
// TODO(copilot): Reimplement using RenderingEngine API - text/font members
// no longer exist in backend-agnostic Text component
Text::Text(Text &&other) noexcept
    : content(std::move(other.content)),
      font_path(std::move(other.font_path)),
      font_id(std::move(other.font_id)),
      character_size(other.character_size),
      color(other.color),
      opacity(other.opacity),
      z_index(other.z_index),
      offset(other.offset),
      is_loaded(other.is_loaded) {
    // No SFML-specific setup needed
}

// Text move assignment
// TODO(copilot): Reimplement using RenderingEngine API
Text &Text::operator=(Text &&other) noexcept {
    if (this == &other)
        return *this;
    content = std::move(other.content);
    font_path = std::move(other.font_path);
    font_id = std::move(other.font_id);
    character_size = other.character_size;
    color = other.color;
    opacity = other.opacity;
    z_index = other.z_index;
    offset = other.offset;
    is_loaded = other.is_loaded;

    return *this;
}

}  // namespace Rtype::Client::Component
