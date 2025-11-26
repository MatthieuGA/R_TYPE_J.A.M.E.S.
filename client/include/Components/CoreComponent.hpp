#pragma once
#include <string>
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct Transform {
    float x;
    float y;
    float rotationDegrees;
    float scale;
};

struct Drawable {
    std::string spritePath;
    int z_index;
    sf::Sprite sprite;
    sf::Texture texture;

    Drawable(const std::string& spritePath, int zIndex)
        : spritePath(spritePath), z_index(zIndex), texture(), sprite(texture) {}
};

struct RigidBody {
    float vx;
    float vy;
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
}  // namespace Rtype::Client::Component
