#pragma once
#include <SFML/Graphics.hpp>

#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Rtype::Client {
namespace Com = Component;
sf::Vector2f GetOffsetFromTransform(
    const Com::Transform &transform, sf::Vector2f hit_box);
sf::Vector2f GetOffsetFromAnimatedTransform(const Com::Transform &transform,
    const Com::AnimatedSprite &animated_sprite);
}  // namespace Rtype::Client
