#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct Transform {
    float x;
    float y;
    float rotationDegrees;
    float scale;

    enum OriginPoint {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        LEFT_CENTER,
        CENTER,
        RIGHT_CENTER,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    } origin = CENTER;

    sf::Vector2f customOrigin = sf::Vector2f(0.0f, 0.0f);
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
};

struct Controllable {
    bool isControllable;
};

struct InputState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
};

struct HitBox {
    float width;
    float height;
    float offsetX;
    float offsetY;
};

struct Solid {
    bool isSolid = true;
    bool isLocked = false;
};

struct Inputs {
    // movement states
    float horizontal = 0.0f;
    float vertical = 0.0f;
    // shoot states
    bool shoot = false;
    bool last_shoot_state = false;
};

}  // namespace Rtype::Client::Component
