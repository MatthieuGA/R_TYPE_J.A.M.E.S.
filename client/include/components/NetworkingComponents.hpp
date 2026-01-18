#pragma once
#include <cstdint>

#include "graphics/Types.hpp"

namespace Rtype::Client::Component {
struct NetworkId {
    int id;
    uint32_t last_processed_tick;
};

struct InterpolatedPosition {
    Engine::Graphics::Vector2f goalPosition;
    float speed;
};
}  // namespace Rtype::Client::Component
