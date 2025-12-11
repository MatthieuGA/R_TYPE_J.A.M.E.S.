#include "engine/OriginTool.hpp"

#include <map>

namespace Rtype::Client {
Engine::Graphics::Vector2f GetOffsetForOrigin(
    const Com::Transform &transform, Engine::Graphics::Vector2f hit_box) {
    std::map<Com::Transform::OriginPoint, Engine::Graphics::Vector2f>
        origin_map = {
            {Com::Transform::TOP_LEFT, Engine::Graphics::Vector2f(0.0f, 0.0f)},
            {Com::Transform::TOP_CENTER,
                Engine::Graphics::Vector2f(hit_box.x / 2.0f, 0.0f)},
            {Com::Transform::TOP_RIGHT,
                Engine::Graphics::Vector2f(hit_box.x, 0.0f)},
            {Com::Transform::LEFT_CENTER,
                Engine::Graphics::Vector2f(0.0f, hit_box.y / 2.0f)},
            {Com::Transform::CENTER, Engine::Graphics::Vector2f(
                                         hit_box.x / 2.0f, hit_box.y / 2.0f)},
            {Com::Transform::RIGHT_CENTER,
                Engine::Graphics::Vector2f(hit_box.x, hit_box.y / 2.0f)},
            {Com::Transform::BOTTOM_LEFT,
                Engine::Graphics::Vector2f(0.0f, hit_box.y)},
            {Com::Transform::BOTTOM_CENTER,
                Engine::Graphics::Vector2f(hit_box.x / 2.0f, hit_box.y)},
            {Com::Transform::BOTTOM_RIGHT,
                Engine::Graphics::Vector2f(hit_box.x, hit_box.y)}};

    Engine::Graphics::Vector2f offset(0.0f, 0.0f);
    if (origin_map.find(transform.origin) != origin_map.end()) {
        offset = Engine::Graphics::Vector2f(
            -origin_map[transform.origin].x, -origin_map[transform.origin].y);
    }
    return offset;
}

Engine::Graphics::Vector2f GetOffsetForCustomOrigin(
    const Com::Transform &transform) {
    return Engine::Graphics::Vector2f(
        -transform.customOrigin.x, -transform.customOrigin.y);
}

Engine::Graphics::Vector2f GetOffsetFromTransform(
    const Com::Transform &transform, Engine::Graphics::Vector2f hitBox) {
    if (transform.customOrigin.x != 0.0f || transform.customOrigin.y != 0.0f) {
        return GetOffsetForCustomOrigin(transform);
    } else {
        return GetOffsetForOrigin(transform, hitBox);
    }
}

Engine::Graphics::Vector2f GetOffsetFromAnimatedTransform(
    const Com::Transform &transform,
    const Com::AnimatedSprite &animated_sprite) {
    return GetOffsetFromTransform(
        transform, Engine::Graphics::Vector2f(
                       static_cast<float>(animated_sprite.frame_width),
                       static_cast<float>(animated_sprite.frame_height)));
}
}  // namespace Rtype::Client
