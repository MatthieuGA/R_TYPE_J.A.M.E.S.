#pragma once
#include <cstdint>

#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct NetworkId {
    int id;
    uint32_t last_processed_tick;
};

struct InterpolatedPosition {
    sf::Vector2f goalPosition;
    float speed;
};
}  // namespace Rtype::Client::Component
