/**
 * @file IVideoModule.hpp
 * @brief Pure virtual interface for video/rendering backend modules.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "graphics/Types.hpp"

namespace Engine {
namespace Video {

// Import graphics types for convenience
using Color = Graphics::Color;
using Vector2f = Graphics::Vector2f;
using Vector2i = Graphics::Vector2i;
using FloatRect = Graphics::FloatRect;
using IntRect = Graphics::IntRect;

/**
 * @brief Transformation matrix (position, rotation, scale).
 */
struct Transform {
    Vector2f position{0.0f, 0.0f};
    float rotation = 0.0f;  // degrees
    Vector2f scale{1.0f, 1.0f};
    Vector2f origin{0.0f, 0.0f};
};

/**
 * @brief Rendering states for draw calls.
 */
struct RenderStates {
    Transform transform;
    const void *texture = nullptr;  // Opaque texture handle
    const void *shader = nullptr;   // Opaque shader handle
    int blend_mode = 0;             // 0=Alpha, 1=Add, 2=Multiply, etc.
};

/**
 * @brief Vertex for custom rendering.
 */
struct Vertex {
    Vector2f position;
    Color color{255, 255, 255, 255};
    Vector2f tex_coords{0.0f, 0.0f};
};

/**
 * @brief Window event types.
 */
enum class EventType {
    CLOSED,
    RESIZED,
    LOST_FOCUS,
    GAINED_FOCUS,
    KEY_PRESSED,
    KEY_RELEASED,
    MOUSE_MOVED,
    MOUSE_BUTTON_PRESSED,
    MOUSE_BUTTON_RELEASED,
    MOUSE_WHEEL_SCROLLED
};

/**
 * @brief Window event data.
 */
struct Event {
    EventType type;

    // Event-specific data
    struct {
        int key_code = 0;
        int mouse_button = 0;
        int mouse_x = 0;
        int mouse_y = 0;
        float mouse_wheel_delta = 0.0f;
        unsigned int width = 0;
        unsigned int height = 0;
    } data;
};

/**
 * @brief Pure virtual interface for video backend modules.
 *
 * All video backend plugins must implement this interface.
 */
class IVideoModule {
 public:
    virtual ~IVideoModule() = default;

    // ===== Lifecycle =====

    /**
     * @brief Initialize the video backend.
     *
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @return true on success, false on failure
     */
    virtual bool Initialize(
        unsigned int width, unsigned int height, const std::string &title) = 0;

    /**
     * @brief Shutdown the video backend.
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Update the backend (process events, etc.).
     *
     * @param delta_time Time elapsed since last update (seconds)
     */
    virtual void Update(float delta_time) = 0;

    // ===== Window Management =====

    /**
     * @brief Check if the window is open.
     *
     * @return true if open, false otherwise
     */
    virtual bool IsWindowOpen() const = 0;

    /**
     * @brief Close the window.
     */
    virtual void CloseWindow() = 0;

    /**
     * @brief Get window size.
     *
     * @return Vector2f with width and height
     */
    virtual Vector2f GetWindowSize() const = 0;

    /**
     * @brief Set window title.
     *
     * @param title New window title
     */
    virtual void SetWindowTitle(const std::string &title) = 0;

    // ===== Event Handling =====

    /**
     * @brief Poll for next event.
     *
     * @param event Output event data
     * @return true if event was retrieved, false if no events
     */
    virtual bool PollEvent(Event &event) = 0;

    // ===== Rendering =====

    /**
     * @brief Clear the screen with a color.
     *
     * @param color Clear color
     */
    virtual void Clear(const Color &color = Color(0, 0, 0, 255)) = 0;

    /**
     * @brief Present the rendered frame.
     */
    virtual void Display() = 0;

    // ===== Texture Management =====

    /**
     * @brief Load a texture from file.
     *
     * @param id Unique identifier
     * @param path File path
     * @return true on success, false on failure
     */
    virtual bool LoadTexture(
        const std::string &id, const std::string &path) = 0;

    /**
     * @brief Get texture handle by ID.
     *
     * @param id Texture identifier
     * @return Opaque texture handle, or nullptr if not found
     */
    virtual const void *GetTexture(const std::string &id) const = 0;

    /**
     * @brief Get texture size.
     *
     * @param id Texture identifier
     * @return Size of texture, or {0, 0} if not found
     */
    virtual Vector2f GetTextureSize(const std::string &id) const = 0;

    // ===== Font Management =====

    /**
     * @brief Load a font from file.
     *
     * @param id Unique identifier
     * @param path File path
     * @return true on success, false on failure
     */
    virtual bool LoadFont(const std::string &id, const std::string &path) = 0;

    /**
     * @brief Get font handle by ID.
     *
     * @param id Font identifier
     * @return Opaque font handle, or nullptr if not found
     */
    virtual const void *GetFont(const std::string &id) const = 0;

    /**
     * @brief Get text bounds for layout calculations.
     *
     * @param text String to measure
     * @param font_id Font identifier
     * @param character_size Font size
     * @return FloatRect with width and height of rendered text
     */
    virtual FloatRect GetTextBounds(const std::string &text,
        const std::string &font_id, unsigned int character_size) const = 0;

    // ===== Sprite Drawing =====

    /**
     * @brief Draw a sprite.
     *
     * @param texture_id Texture identifier
     * @param transform Transformation (position, rotation, scale)
     * @param texture_rect Source rectangle in texture (optional)
     * @param color Tint color
     * @param shader_id Shader identifier (optional)
     */
    virtual void DrawSprite(const std::string &texture_id,
        const Transform &transform, const FloatRect *texture_rect = nullptr,
        const Color &color = Color(255, 255, 255, 255),
        const std::string *shader_id =
            nullptr) = 0;  // ===== Text Drawing =====

    /**
     * @brief Draw text.
     *
     * @param text String to draw
     * @param font_id Font identifier
     * @param transform Transformation (position, rotation, scale, origin)
     * @param character_size Font size
     * @param color Text color
     */
    virtual void DrawText(const std::string &text, const std::string &font_id,
        const Transform &transform, unsigned int character_size,
        const Color &color = Color(255, 255, 255, 255)) = 0;

    // ===== Primitive Drawing =====

    /**
     * @brief Draw a rectangle.
     *
     * @param rect Rectangle definition
     * @param color Fill color
     * @param outline_color Outline color (optional)
     * @param outline_thickness Outline thickness (optional)
     */
    virtual void DrawRectangle(const FloatRect &rect, const Color &color,
        const Color *outline_color = nullptr,
        float outline_thickness = 0.0f) = 0;

    /**
     * @brief Draw a circle.
     *
     * @param center Center position
     * @param radius Circle radius
     * @param color Fill color
     * @param outline_color Outline color (optional)
     * @param outline_thickness Outline thickness (optional)
     */
    virtual void DrawCircle(const Vector2f &center, float radius,
        const Color &color, const Color *outline_color = nullptr,
        float outline_thickness = 0.0f) = 0;

    // ===== Advanced =====

    /**
     * @brief Draw custom vertices.
     *
     * @param vertices Vertex array
     * @param vertex_count Number of vertices
     * @param primitive_type 0=Points, 1=Lines, 2=Triangles, 3=Quads
     * @param states Render states
     */
    virtual void DrawVertices(const Vertex *vertices, size_t vertex_count,
        int primitive_type, const RenderStates &states) = 0;

    // ===== Shader Management =====

    /**
     * @brief Load a shader from file.
     *
     * @param id Unique identifier
     * @param vertex_path Vertex shader path (optional)
     * @param fragment_path Fragment shader path (optional)
     * @return true on success, false on failure
     */
    virtual bool LoadShader(const std::string &id,
        const std::string &vertex_path, const std::string &fragment_path) = 0;

    /**
     * @brief Set shader parameter.
     *
     * @param shader_id Shader identifier
     * @param name Parameter name
     * @param value Parameter value (float)
     */
    virtual void SetShaderParameter(const std::string &shader_id,
        const std::string &name, float value) = 0;

    // ===== Metadata =====

    /**
     * @brief Get module name.
     *
     * @return Human-readable module name
     */
    virtual std::string GetModuleName() const = 0;

    // ===== Compatibility Bridge =====

    /**
     * @brief Get underlying platform-specific window handle.
     *
     * This is a compatibility bridge for legacy code. Returns nullptr
     * if not applicable to the backend implementation.
     *
     * @return Opaque pointer to platform window (e.g., sf::RenderWindow*)
     */
    virtual void *GetNativeWindow() const = 0;
};

}  // namespace Video
}  // namespace Engine
