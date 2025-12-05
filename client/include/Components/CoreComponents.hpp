#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <optional>
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
/**
 * @brief Transform component for hierarchical positioning and rotation.
 *
 * Uses Entity IDs for parent-child relationships instead of raw pointers
 * to avoid dangling pointer issues when the sparse_array reallocates.
 */
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

    // Parent entity ID (std::nullopt if no parent)
    std::optional<std::size_t> parent_entity = std::nullopt;

    Transform() = default;
    Transform(float x, float y, float rotationDegrees, float scale,
        OriginPoint origin = CENTER,
        sf::Vector2f customOrigin = sf::Vector2f(0.0f, 0.0f),
        std::optional<std::size_t> parent_entity = std::nullopt)
    : x(x), y(y), rotationDegrees(rotationDegrees), scale(scale),
        origin(origin), customOrigin(customOrigin),
        parent_entity(parent_entity) {}

    /**
     * @brief Gets the cumulative rotation including parent rotations.
     * @note This only uses local rotation; parent rotations must be added by the render system.
     */
    float GetWorldRotation() const {
        return rotationDegrees;
    }
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
