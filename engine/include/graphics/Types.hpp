/**
 * @file Types.hpp
 * @brief Engine-agnostic graphics types.
 *
 * These types decouple the engine from any specific graphics library (SFML,
 * SDL, etc.). Plugins convert between these types and their native types
 * internally.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_TYPES_HPP_
#define ENGINE_INCLUDE_GRAPHICS_TYPES_HPP_

#include <cstdint>

namespace Engine {
namespace Graphics {

/**
 * @brief 2D vector with float components.
 */
struct Vector2f {
    float x;
    float y;

    Vector2f() : x(0.0f), y(0.0f) {}

    Vector2f(float x_val, float y_val) : x(x_val), y(y_val) {}

    Vector2f operator+(const Vector2f &other) const {
        return Vector2f(x + other.x, y + other.y);
    }

    Vector2f operator-(const Vector2f &other) const {
        return Vector2f(x - other.x, y - other.y);
    }

    Vector2f operator*(float scalar) const {
        return Vector2f(x * scalar, y * scalar);
    }

    Vector2f operator*(const Vector2f &other) const {
        return Vector2f(x * other.x, y * other.y);
    }

    Vector2f operator/(float scalar) const {
        return Vector2f(x / scalar, y / scalar);
    }

    Vector2f &operator+=(const Vector2f &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2f &operator-=(const Vector2f &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

/**
 * @brief 2D vector with integer components.
 */
struct Vector2i {
    int x;
    int y;

    Vector2i() : x(0), y(0) {}

    Vector2i(int x_val, int y_val) : x(x_val), y(y_val) {}
};

/**
 * @brief RGBA color with 8-bit components.
 */
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color() : r(255), g(255), b(255), a(255) {}

    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    // Predefined colors
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Transparent;
};

/**
 * @brief Integer rectangle.
 */
struct IntRect {
    int left;
    int top;
    int width;
    int height;

    IntRect() : left(0), top(0), width(0), height(0) {}

    IntRect(int l, int t, int w, int h)
        : left(l), top(t), width(w), height(h) {}
};

/**
 * @brief Float rectangle.
 */
struct FloatRect {
    float left;
    float top;
    float width;
    float height;

    FloatRect() : left(0.0f), top(0.0f), width(0.0f), height(0.0f) {}

    FloatRect(float l, float t, float w, float h)
        : left(l), top(t), width(w), height(h) {}

    bool Contains(const Vector2f &point) const {
        return point.x >= left && point.x <= (left + width) &&
               point.y >= top && point.y <= (top + height);
    }
};

/**
 * @brief Opaque handle for textures managed by the video backend.
 */
using TextureHandle = uint32_t;
constexpr TextureHandle INVALID_TEXTURE = 0;

/**
 * @brief Opaque handle for fonts managed by the video backend.
 */
using FontHandle = uint32_t;
constexpr FontHandle INVALID_FONT = 0;

/**
 * @brief Opaque handle for shaders managed by the video backend.
 */
using ShaderHandle = uint32_t;
constexpr ShaderHandle INVALID_SHADER = 0;

/**
 * @brief Text alignment options.
 */
enum class TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

/**
 * @brief Text style flags (can be combined).
 */
enum class TextStyle : uint32_t {
    REGULAR = 0,
    BOLD = 1 << 0,
    ITALIC = 1 << 1,
    UNDERLINED = 1 << 2,
    STRIKETHROUGH = 1 << 3
};

inline TextStyle operator|(TextStyle a, TextStyle b) {
    return static_cast<TextStyle>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline TextStyle operator&(TextStyle a, TextStyle b) {
    return static_cast<TextStyle>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

}  // namespace Graphics
}  // namespace Engine

#endif  // ENGINE_INCLUDE_GRAPHICS_TYPES_HPP_  // ENGINE_GRAPHICS_TYPES_HPP_
