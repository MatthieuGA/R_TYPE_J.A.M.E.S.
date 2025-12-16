#pragma once
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct NetworkId {
    int id;
    int last_processed_tick;
};

struct InterpolatedPosition {
    sf::Vector2f goalPosition;
    float speed;
};
}  // namespace Rtype::Client::Component
