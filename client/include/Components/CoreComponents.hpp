#pragma once
#include <string>
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct Transform {
    float x;
    float y;
    float rotationDegrees;
    float scale;
    enum OriginPoint {
        TOP_LEFT,
        CENTER
    } origin = TOP_LEFT;
};

struct Drawable {
    std::string spritePath;
    int z_index;
    sf::Sprite sprite;
    sf::Texture texture;
    bool isLoaded = false;

    Drawable(const std::string& spritePath, int zIndex)
        : spritePath("Assets/" + spritePath), z_index(zIndex), texture(),
        sprite(texture), isLoaded(false) {}
};

struct AnimatedSprite {
    int frameWidth;
    int frameHeight;
    int totalFrames;
    int currentFrame;
    float frameDuration;
    float elapsedTime;
    bool loop;
};

struct Velocity {
    float vx;
    float vy;
    float accelerationX;
    float accelerationY;
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
