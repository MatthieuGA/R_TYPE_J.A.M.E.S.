
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

#include <vector>

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
    /// Source rectangle for texture cropping (animation frames, etc.)
    /// {0, 0, 0, 0} means use full texture; otherwise crop to this rect
    IntRect source_rect;
    /// Origin point for sprite (local coordinates where sprite pivots)
    /// Used for centering and rotation
    Vector2f origin;
};

/**
 * @brief Shader data passed to the render backend.
 *
 * Contains the shader path and a list of float uniforms to apply
 * before drawing. The backend is responsible for loading/caching
 * the shader implementation and setting these uniforms.
 */
struct DrawableShader {
    struct Uniform {
        const char *name;
        float value;
    };

    /// Path to shader file (backend-resolvable, typically prefixed)
    const char *shader_path;
    /// Float uniforms to apply (re-applied each draw)
    std::vector<Uniform> float_uniforms;
    /// Time in seconds for animated shaders (e.g., sine/cos effects)
    float time_seconds = 0.0f;
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
    /// Scale factors (1.0 = original size)
    /// SFML text scaling; default is {1.0, 1.0}
    Vector2f scale;
    /// Origin point for text (local coordinates where text pivots)
    /// Used for centering and rotation
    Vector2f origin;
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
        LineStrip = 2,
        Triangles = 3,
        TriangleStrip = 4,
        TriangleFan = 5,
        Quads = 6
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
    virtual void DrawSprite(const DrawableSprite &sprite,
        const DrawableShader *shader = nullptr) = 0;

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
