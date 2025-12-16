#pragma once
#include <SFML/Graphics.hpp>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Vector2f.hpp"

namespace server {
vector2f GetOffsetFromTransform(
    const Component::Transform &transform, vector2f hit_box);
vector2f GetOffsetFromAnimatedTransform(const Component::Transform &transform,
    const Component::AnimatedSprite &animated_sprite);
}  // namespace server
