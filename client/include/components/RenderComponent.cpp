#include "include/components/RenderComponent.hpp"

#include <string>
#include <utility>
#include <vector>

namespace Rtype::Client::Component {

// AnimatedSprite constructors
AnimatedSprite::AnimatedSprite(int frameWidth, int frameHeight,
    float frameDuration, bool loop, sf::Vector2f first_frame_position,
    int totalFrames)
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
    float frameDuration, bool loop, sf::Vector2f first_frame_position,
    sf::Vector2f offset) {
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

std::vector<std::string> AnimatedSprite::GetAnimationNames() const {
    std::vector<std::string> names;
    for (const auto &[name, animation] : animations) {
        names.push_back(name);
    }
    return names;
}

// Text move constructor
Text::Text(Text &&other) noexcept
    : content(std::move(other.content)),
      fontPath(std::move(other.fontPath)),
      characterSize(other.characterSize),
      color(other.color),
      opacity(other.opacity),
      z_index(other.z_index),
      offset(other.offset),
      text(),
      font(std::move(other.font)),
      is_loaded(other.is_loaded) {
    text.setFont(font);
    text.setString(other.text.getString());
    text.setCharacterSize(other.text.getCharacterSize());
    text.setFillColor(other.text.getFillColor());
}

// Text move assignment
Text &Text::operator=(Text &&other) noexcept {
    if (this == &other)
        return *this;
    content = std::move(other.content);
    fontPath = std::move(other.fontPath);
    characterSize = other.characterSize;
    color = other.color;
    opacity = other.opacity;
    z_index = other.z_index;
    offset = other.offset;
    font = std::move(other.font);
    is_loaded = other.is_loaded;

    text = sf::Text();
    text.setFont(font);
    text.setString(other.text.getString());
    text.setCharacterSize(other.text.getCharacterSize());
    text.setFillColor(other.text.getFillColor());

    return *this;
}

}  // namespace Rtype::Client::Component
