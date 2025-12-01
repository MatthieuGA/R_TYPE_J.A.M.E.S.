#pragma once
#include <SFML/Graphics.hpp>
#include "include/Components/CoreComponents.hpp"

namespace Rtype::Client {
namespace Com = Component;
sf::Vector2f get_offset_from_transform(const Com::Transform &transform,
    sf::Vector2f hitBox);
sf::Vector2f get_offset_from_animated_transform(const Com::Transform &transform,
    const Com::AnimatedSprite &animatedSprite);
}  // namespace Rtype::Client