#pragma once
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct NetworkId {
    int id;
};

struct InterpolatedPosition {
    sf::Vector2f goalPosition;
    float speed;
};
}  // namespace Rtype::Client::Component
