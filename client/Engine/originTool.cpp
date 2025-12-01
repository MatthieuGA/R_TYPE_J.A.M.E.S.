#include "Engine/originTool.hpp"

namespace Rtype::Client {
sf::Vector2f get_offset_for_origin(const Com::Transform &transform,
    sf::Vector2f hitBox) {
    sf::Vector2f offset(0.0f, 0.0f);

    std::map<Com::Transform::OriginPoint, sf::Vector2f> originMap = {
        {Com::Transform::TOP_LEFT, sf::Vector2f(0.0f, 0.0f)},
        {Com::Transform::TOP_CENTER, sf::Vector2f(hitBox.x / 2.0f, 0.0f)},
        {Com::Transform::TOP_RIGHT, sf::Vector2f(hitBox.x, 0.0f)},
        {Com::Transform::LEFT_CENTER, sf::Vector2f(0.0f, hitBox.y / 2.0f)},
        {Com::Transform::CENTER, sf::Vector2f(hitBox.x / 2.0f, hitBox.y / 2.0f)},
        {Com::Transform::RIGHT_CENTER, sf::Vector2f(hitBox.x, hitBox.y / 2.0f)},
        {Com::Transform::BOTTOM_LEFT, sf::Vector2f(0.0f, hitBox.y)},
        {Com::Transform::BOTTOM_CENTER, sf::Vector2f(hitBox.x / 2.0f, hitBox.y)},
        {Com::Transform::BOTTOM_RIGHT, sf::Vector2f(hitBox.x, hitBox.y)}
    };

    if (originMap.find(transform.origin) != originMap.end()) {
        offset = -originMap[transform.origin];
    }
    return offset;
}

sf::Vector2f get_offset_for_custom_origin(const Com::Transform &transform) {
    return sf::Vector2f(-transform.customOrigin.x, -transform.customOrigin.y);
}

sf::Vector2f get_offset_from_transform(const Com::Transform &transform,
    sf::Vector2f hitBox) {
    if (transform.customOrigin != sf::Vector2f(0.0f, 0.0f)) {
        return get_offset_for_custom_origin(transform);
    } else {
        return get_offset_for_origin(transform, hitBox);
    }
}

sf::Vector2f get_offset_from_animated_transform(const Com::Transform &transform,
    const Com::AnimatedSprite &animatedSprite) {
    return get_offset_from_transform(transform, sf::Vector2f(
        static_cast<float>(animatedSprite.frameWidth),
        static_cast<float>(animatedSprite.frameHeight)));
}
}  // namespace Rtype::Client