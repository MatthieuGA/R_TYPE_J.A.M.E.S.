#pragma once
#include <graphics/Types.hpp>

namespace Rtype::Client::Component {
struct NetworkId {
    int id;
};

struct InterpolatedPosition {
    Engine::Graphics::Vector2f goalPosition;
    float speed;
};
}  // namespace Rtype::Client::Component
