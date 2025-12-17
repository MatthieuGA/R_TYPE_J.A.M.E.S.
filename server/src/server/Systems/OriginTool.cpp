#include <map>

#include "server/systems/CollidingTools.hpp"

namespace server {
vector2f GetOffsetForOrigin(
    const Component::Transform &transform, vector2f hit_box) {
    std::map<Component::Transform::OriginPoint, vector2f> origin_map = {
        {Component::Transform::TOP_LEFT, vector2f(0.0f, 0.0f)},
        {Component::Transform::TOP_CENTER, vector2f(hit_box.x / 2.0f, 0.0f)},
        {Component::Transform::TOP_RIGHT, vector2f(hit_box.x, 0.0f)},
        {Component::Transform::LEFT_CENTER, vector2f(0.0f, hit_box.y / 2.0f)},
        {Component::Transform::CENTER,
            vector2f(hit_box.x / 2.0f, hit_box.y / 2.0f)},
        {Component::Transform::RIGHT_CENTER,
            vector2f(hit_box.x, hit_box.y / 2.0f)},
        {Component::Transform::BOTTOM_LEFT, vector2f(0.0f, hit_box.y)},
        {Component::Transform::BOTTOM_CENTER,
            vector2f(hit_box.x / 2.0f, hit_box.y)},
        {Component::Transform::BOTTOM_RIGHT, vector2f(hit_box.x, hit_box.y)}};

    vector2f offset(0.0f, 0.0f);
    if (origin_map.find(transform.origin) != origin_map.end()) {
        offset = {
            -origin_map[transform.origin].x, -origin_map[transform.origin].y};
    }
    return offset;
}

vector2f GetOffsetForCustomOrigin(const Component::Transform &transform) {
    return vector2f(-transform.customOrigin.x, -transform.customOrigin.y);
}

vector2f GetOffsetFromTransform(
    const Component::Transform &transform, vector2f hitBox) {
    if (transform.customOrigin.x != 0.0f || transform.customOrigin.y != 0.0f) {
        return GetOffsetForCustomOrigin(transform);
    } else {
        return GetOffsetForOrigin(transform, hitBox);
    }
}
}  // namespace server
