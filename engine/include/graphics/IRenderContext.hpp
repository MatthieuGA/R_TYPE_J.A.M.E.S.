
/**
 * @file IRenderContext.hpp
 * @brief Engine-level rendering interface (backend-agnostic).
 *
 * This interface defines rendering operations without exposing backend
 * specifics (SFML, SDL, OpenGL, etc.). It uses only engine types.
 *
 * SCOPE: PR 1.8.1 - Interface + SFML backend implementation ONLY.
 * NO systems, components, or game logic modifications.
 * Must preserve exact current rendering behavior.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_IRENDERCONTEXT_HPP_
#define ENGINE_INCLUDE_GRAPHICS_IRENDERCONTEXT_HPP_

#include "graphics/Types.hpp"

namespace Engine {
namespace Graphics {

/**
 * @brief Drawable sprite data (backend-agnostic).
 *
 * Represents sprite rendering intent without backend-specific state.
 * Backend translates this to native draw calls.
 */
struct DrawableSprite {
    /// Path to texture file
    const char *texture_path;
    /// Position in pixels
    Vector2f position;
    /// Scale factors (1.0 = original size)
    Vector2f scale;
    /// Rotation in degrees (clockwise)
    float rotation_degrees;
    /// Color tint with alpha
    Color color;
};

/**
 * @brief Drawable text data (backend-agnostic).
 *
 * Represents text rendering intent without backend-specific state.
 */
struct DrawableText {
    /// Path to font file
    const char *font_path;
    /// Text string to display
    const char *text;
    /// Character size in pixels
    unsigned int size;
    /// Position in pixels
    Vector2f position;
    /// Color with alpha
    Color color;
};

/**
 * @brief Drawable rectangle data (backend-agnostic).
 *
 * Represents filled rectangle rendering.
 */
struct DrawableRectangle {
    /// Position in pixels
    Vector2f position;
    /// Size in pixels
    Vector2f size;
    /// Fill color with alpha
    Color color;
};

/**
 * @brief Vertex array for custom shapes.
 *
 * Allows rendering arbitrary geometry.
 */
struct VertexArray {
    /**
     * @brief Single vertex with position and color.
     */
    struct Vertex {
        Vector2f position;
        Color color;
    };

    /**
     * @brief Primitive type for vertex array.
     */
    enum PrimitiveType {
        Points = 0,
        Lines = 1,
        LineStrip = 3,
        Triangles = 4,
        TriangleStrip = 5,
        TriangleFan = 6,
        Quads = 7
    };

    /// Array of vertices
    const Vertex *vertices;
    /// Number of vertices
    size_t vertex_count;
    /// How to interpret vertices
    PrimitiveType primitive_type;
};

/**
 * @brief Abstract rendering context interface.
 *
 * Defines drawing operations that backends must implement.
 * All drawing goes through this interface - no direct backend calls.
 *
 * CRITICAL: This is a MINIMAL interface reflecting CURRENT usage.
 * DO NOT add features that aren't actively used today.
 */
class IRenderContext {
 public:
    virtual ~IRenderContext() = default;

    /**
     * @brief Draw a sprite.
     *
     * @param sprite Sprite data to draw
     * @param shader_ptr Optional shader (backend-specific, can be nullptr)
     */
    virtual void DrawSprite(
        const DrawableSprite &sprite, void *shader_ptr = nullptr) = 0;

    /**
     * @brief Draw text.
     *
     * @param text Text data to draw
     */
    virtual void DrawText(const DrawableText &text) = 0;

    /**
     * @brief Draw a filled rectangle.
     *
     * @param rect Rectangle data to draw
     */
    virtual void DrawRectangle(const DrawableRectangle &rect) = 0;

    /**
     * @brief Draw a vertex array.
     *
     * @param vertices Vertex array data
     */
    virtual void DrawVertexArray(const VertexArray &vertices) = 0;
};

}  // namespace Graphics
}  // namespace Engine

#endif  // ENGINE_INCLUDE_GRAPHICS_IRENDERCONTEXT_HPP_
