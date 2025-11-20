#pragma once
#include <string>

namespace Rtype::Client::Component {
struct position {
    float x;
    float y;
};
struct velocity {
    float vx;
    float vy;
};
struct drawable {
    std::string sprite;
    float scale;
};
struct controllable {
    bool isControllable;
};
} // namespace Component
