#pragma once
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <limits>

#include <SFML/Graphics.hpp>
#include <graphics/Types.hpp>

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
    Engine::Graphics::Vector2f scale;

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

    Engine::Graphics::Vector2f customOrigin{0.0f, 0.0f};

    // Parent entity ID (std::nullopt if no parent)
    std::optional<std::size_t> parent_entity = std::nullopt;

    // List of child entity IDs for hierarchical relationships
    std::vector<std::size_t> children;

    Transform() = default;

    Transform(float x, float y, float rotationDegrees,
        Engine::Graphics::Vector2f scale, OriginPoint origin = CENTER,
        Engine::Graphics::Vector2f customOrigin = Engine::Graphics::Vector2f(
            0.0f, 0.0f),
        std::optional<std::size_t> parent_entity = std::nullopt)
        : x(x),
          y(y),
          rotationDegrees(rotationDegrees),
          scale(scale),
          origin(origin),
          customOrigin(customOrigin),
          parent_entity(parent_entity),
          children() {}

    Transform(float x, float y, float rotationDegrees, float scale,
        OriginPoint origin = CENTER,
        Engine::Graphics::Vector2f customOrigin = Engine::Graphics::Vector2f(
            0.0f, 0.0f),
        std::optional<std::size_t> parent_entity = std::nullopt)
        : x(x),
          y(y),
          rotationDegrees(rotationDegrees),
          scale({scale, scale}),
          origin(origin),
          customOrigin(customOrigin),
          parent_entity(parent_entity),
          children() {}

    /**
     * @brief Gets the cumulative rotation including parent rotations.
     * @note This only uses local rotation; parent rotations must be added by
     * the render system.
     */
    float GetWorldRotation() const;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
};

struct Controllable {
    bool isControllable = true;
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
    bool scaleWithTransform = true;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
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

struct Clickable {
    std::function<void()> onClick;

    Engine::Graphics::Color idleColor = Engine::Graphics::Color::White;
    Engine::Graphics::Color hoverColor =
        Engine::Graphics::Color(200, 200, 200);
    Engine::Graphics::Color clickColor =
        Engine::Graphics::Color(150, 150, 150);
    bool isHovered = false;
    bool isClicked = false;
};

/**
 * @brief Component for draggable UI elements like sliders.
 *
 * Allows entities to be dragged with mouse input. Tracks drag state,
 * constraints, and provides callbacks for drag events.
 */
struct Draggable {
    bool is_dragging =
        false; /**< Whether element is currently being dragged */
    Engine::Graphics::Vector2f drag_offset{
        0.0f, 0.0f}; /**< Offset from drag start position */

    // Drag constraints
    bool constrain_horizontal =
        false; /**< Restrict dragging to horizontal axis */
    bool constrain_vertical = false; /**< Restrict dragging to vertical axis */
    float min_x = -std::numeric_limits<float>::infinity();
    float max_x = std::numeric_limits<float>::infinity();
    float min_y = -std::numeric_limits<float>::infinity();
    float max_y = std::numeric_limits<float>::infinity();

    // Callbacks
    std::function<void(float, float)> on_drag;
    std::function<void(float, float)> on_drag_start;
    std::function<void(float, float)> on_drag_end;

    /**
     * @brief Constructor with horizontal constraint and range.
     *
     * @param min_x_pos Minimum X position for dragging.
     * @param max_x_pos Maximum X position for dragging.
     */
    Draggable(float min_x_pos, float max_x_pos)
        : constrain_horizontal(false),
          constrain_vertical(true),
          min_x(min_x_pos),
          max_x(max_x_pos) {}

    Draggable() = default;
};

struct SoundRequest {
    std::string sound_id;
    float volume = 1.0f;
    bool loop = false;
};

}  // namespace Rtype::Client::Component
