/**
 * @file RenderingEngine.hpp
 * @brief High-level rendering engine that provides game-level abstractions
 * over low-level video plugins.
 */

#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "graphics/Types.hpp"
#include "video/IVideoModule.hpp"

namespace Engine {
namespace Rendering {

// Import commonly used types
using Graphics::Color;
using Graphics::FloatRect;
using Graphics::IntRect;
using Graphics::Vector2f;
using Video::Transform;

/**
 * @brief Camera for viewport management and transformations.
 */
struct Camera {
    Vector2f position{0.0f, 0.0f};
    float zoom = 1.0f;
    Vector2f size{1920.0f, 1080.0f};  // Viewport size

    /**
     * @brief Convert world coordinates to screen coordinates.
     * Uses top-left origin (no centering offset) to match original coordinate system.
     */
    Vector2f WorldToScreen(const Vector2f &world_pos) const {
        Vector2f screen;
        screen.x = (world_pos.x - position.x) * zoom;
        screen.y = (world_pos.y - position.y) * zoom;
        return screen;
    }
};

/**
 * @brief High-level rendering engine for game entities.
 *
 * Provides game-level abstractions over the low-level IVideoModule plugin
 * interface. Handles common rendering tasks like transform hierarchy,
 * z-ordering, shader management, and resource loading.
 */
class RenderingEngine {
 public:
    /**
     * @brief Construct rendering engine with a video plugin.
     *
     * @param plugin The video module plugin to use for rendering
     */
    explicit RenderingEngine(std::shared_ptr<Video::IVideoModule> plugin);
    ~RenderingEngine() = default;

    // ===== Lifecycle =====

    /**
     * @brief Initialize the rendering engine.
     *
     * @param width Window width
     * @param height Window height
     * @param title Window title
     * @return true on success
     */
    bool Initialize(
        unsigned int width, unsigned int height, const std::string &title);

    /**
     * @brief Shutdown the rendering engine.
     */
    void Shutdown();

    /**
     * @brief Update per frame (for animations, time-based effects).
     *
     * @param delta_time Time elapsed since last frame
     */
    void Update(float delta_time);

    // ===== Window Management =====

    bool IsWindowOpen() const;
    void CloseWindow();
    Vector2f GetWindowSize() const;
    void SetWindowTitle(const std::string &title);

    // ===== Event Handling =====

    bool PollEvent(Video::Event &event);

    // ===== Frame Management =====

    /**
     * @brief Begin a new frame (clear screen).
     *
     * @param clear_color Background color
     */
    void BeginFrame(const Color &clear_color = Color(30, 30, 80, 255));

    /**
     * @brief End frame and present to screen.
     */
    void EndFrame();

    // ===== Camera =====

    void SetCamera(const Camera &camera);
    const Camera &GetCamera() const;
    Camera &GetCamera();

    // ===== High-Level Entity Rendering =====

    /**
     * @brief Render a sprite entity with full transform hierarchy support.
     *
     * Handles:
     * - Transform hierarchy (parent-child relationships)
     * - Texture rect for sprite sheets
     * - Color tinting and opacity
     * - Origin calculation
     * - Shader application
     * - Z-index ordering (when used with layers)
     *
     * @param texture_id Texture identifier
     * @param world_position Final world position (after hierarchy)
     * @param world_scale Cumulative scale from hierarchy
     * @param rotation Rotation in degrees
     * @param texture_rect Source rectangle in texture (nullptr for full
     * texture)
     * @param color Tint color with alpha
     * @param origin_offset Origin offset for rotation/scaling
     * @param shader_id Optional shader to apply
     */
    void RenderSprite(const std::string &texture_id,
        const Vector2f &world_position, const Vector2f &world_scale, float rotation,
        const FloatRect *texture_rect, const Color &color,
        const Vector2f &origin_offset, const std::string *shader_id);

    /**
     * @brief Render text with transform.
     *
     * @param text String to render
     * @param font_id Font identifier
     * @param world_position World position
     * @param world_scale Scale factor
     * @param rotation Rotation in degrees
     * @param character_size Font size
     * @param color Text color
     * @param origin_offset Origin offset
     */
    void RenderText(const std::string &text, const std::string &font_id,
        const Vector2f &world_position, float world_scale, float rotation,
        unsigned int character_size, const Color &color,
        const Vector2f &origin_offset);

    /**
     * @brief Render a batch of particles efficiently.
     *
     * Uses instanced rendering or optimized batch drawing for particles.
     * Particles are rendered as colored points/quads.
     *
     * @param particles Vector of particle positions
     * @param colors Vector of particle colors (same size as positions)
     * @param sizes Vector of particle sizes (same size as positions)
     * @param z_index Z-index for rendering order
     */
    void RenderParticles(const std::vector<Vector2f> &particles,
        const std::vector<Color> &colors, const std::vector<float> &sizes,
        int z_index = 0);

    // ===== Resource Management =====

    bool LoadTexture(const std::string &id, const std::string &path);
    bool UnloadTexture(const std::string &id);  // Reference-counted unload
    bool LoadFont(const std::string &id, const std::string &path);
    bool LoadShader(const std::string &id, const std::string &vertex_path,
        const std::string &fragment_path);

    Vector2f GetTextureSize(const std::string &id) const;
    FloatRect GetTextBounds(const std::string &text,
        const std::string &font_id, unsigned int character_size) const;

    // ===== Shader Management =====

    /**
     * @brief Set shader uniform parameter.
     *
     * @param shader_id Shader identifier
     * @param name Uniform name
     * @param value Uniform value
     */
    void SetShaderParameter(
        const std::string &shader_id, const std::string &name, float value);

    // ===== Low-Level Primitive Drawing =====

    void DrawRectangle(const FloatRect &rect, const Color &color,
        const Color *outline_color = nullptr, float outline_thickness = 0.0f);

    void DrawCircle(const Vector2f &center, float radius, const Color &color,
        const Color *outline_color = nullptr, float outline_thickness = 0.0f);

    // ===== Metadata =====

    std::string GetModuleName() const;

    // ===== Direct Plugin Access (for advanced use cases) =====

    /**
     * @brief Get raw video module pointer.
     *
     * Use with caution; prefer using RenderingEngine methods.
     *
     * @return Raw pointer to video module
     */
    Video::IVideoModule *GetPlugin() const;

    /**
     * @brief Get platform-specific window handle (SFML, SDL, etc.).
     *
     * For legacy compatibility.
     */
    void *GetNativeWindow() const;

 private:
    std::shared_ptr<Video::IVideoModule> plugin_;
    Camera camera_;
    float accumulated_time_ = 0.0f;
};

}  // namespace Rendering
}  // namespace Engine
