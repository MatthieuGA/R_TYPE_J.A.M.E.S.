#include "engine/OriginTool.hpp"

#include <map>

namespace Rtype::Client {
sf::Vector2f GetOffsetForOrigin(
    const Com::Transform &transform, sf::Vector2f hit_box) {
    std::map<Com::Transform::OriginPoint, sf::Vector2f> origin_map = {
        {Com::Transform::TOP_LEFT, sf::Vector2f(0.0f, 0.0f)},
        {Com::Transform::TOP_CENTER, sf::Vector2f(hit_box.x / 2.0f, 0.0f)},
        {Com::Transform::TOP_RIGHT, sf::Vector2f(hit_box.x, 0.0f)},
        {Com::Transform::LEFT_CENTER, sf::Vector2f(0.0f, hit_box.y / 2.0f)},
        {Com::Transform::CENTER,
            sf::Vector2f(hit_box.x / 2.0f, hit_box.y / 2.0f)},
        {Com::Transform::RIGHT_CENTER,
            sf::Vector2f(hit_box.x, hit_box.y / 2.0f)},
        {Com::Transform::BOTTOM_LEFT, sf::Vector2f(0.0f, hit_box.y)},
        {Com::Transform::BOTTOM_CENTER,
            sf::Vector2f(hit_box.x / 2.0f, hit_box.y)},
        {Com::Transform::BOTTOM_RIGHT, sf::Vector2f(hit_box.x, hit_box.y)}};

    sf::Vector2f offset(0.0f, 0.0f);
    if (origin_map.find(transform.origin) != origin_map.end()) {
        offset = -origin_map[transform.origin];
    }
    return offset;
}

sf::Vector2f GetOffsetForCustomOrigin(const Com::Transform &transform) {
    return sf::Vector2f(-transform.customOrigin.x, -transform.customOrigin.y);
}

sf::Vector2f GetOffsetFromTransform(
    const Com::Transform &transform, sf::Vector2f hitBox) {
    if (transform.customOrigin != sf::Vector2f(0.0f, 0.0f)) {
        return GetOffsetForCustomOrigin(transform);
    } else {
        return GetOffsetForOrigin(transform, hitBox);
    }
}

sf::Vector2f GetOffsetFromAnimatedTransform(const Com::Transform &transform,
    const Com::AnimatedSprite &animated_sprite) {
    return GetOffsetFromTransform(
        transform, sf::Vector2f(static_cast<float>(animated_sprite.frameWidth),
                       static_cast<float>(animated_sprite.frameHeight)));
}
}  // namespace Rtype::Client
