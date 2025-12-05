#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
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

struct Drawable {
    std::string spritePath;
    int z_index = 0;
    float opacity = 1.0f;
    sf::Sprite sprite;
    sf::Texture texture;
    bool isLoaded = false;

    explicit Drawable(const std::string& spritePath, int zIndex = 0,
        float opacity = 1.0f)
        : spritePath("Assets/Images/" + spritePath), z_index(zIndex), texture(),
        opacity(opacity), sprite(texture), isLoaded(false) {}
};

struct Shader {
    std::string shaderPath;
    std::shared_ptr<sf::Shader> shader;
    bool isLoaded = false;
    std::map<std::string, float> uniforms_float = {};

    explicit Shader(const std::string& path,
        std::vector<std::pair<std::string, float>> uf = {})
        : shaderPath("Assets/Shaders/" + path), shader(nullptr),
        isLoaded(false) {
        for (const auto& [name, value] : uf)
            uniforms_float[name] = value;
    }
};

struct AnimatedSprite {
    int frameWidth;
    int frameHeight;
    int totalFrames;
    int currentFrame = 0;
    float frameDuration = 0.1f;
    bool loop = true;
    float elapsedTime = 0.0f;
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
}  // namespace Rtype::Client::Component
