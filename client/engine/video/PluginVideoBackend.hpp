/**
 * @file PluginVideoBackend.hpp
 * @brief Adapter between video plugin and game engine.
 */

#pragma once
#include <memory>
#include <string>

#include "video/IVideoModule.hpp"

namespace Engine {
namespace Video {

/**
 * @brief Adapter that wraps a video plugin for use by the engine.
 *
 * This class provides a simplified interface that the engine
 * can use without directly depending on plugin implementation details.
 */
class PluginVideoBackend {
 public:
    /**
     * @brief Construct adapter with a video plugin.
     *
     * @param module Shared pointer to video module plugin
     */
    explicit PluginVideoBackend(std::shared_ptr<IVideoModule> module);

    ~PluginVideoBackend() = default;

    // ===== Lifecycle =====

    bool Initialize(
        unsigned int width, unsigned int height, const std::string &title);
    void Shutdown();
    void Update(float delta_time);

    // ===== Window Management =====

    bool IsWindowOpen() const;
    void CloseWindow();
    Vector2f GetWindowSize() const;
    void SetWindowTitle(const std::string &title);

    // ===== Event Handling =====

    bool PollEvent(Event &event);

    // ===== Rendering =====

    void Clear(const Color &color = Color(0, 0, 0, 255));
    void Display();

    // ===== Resource Management =====

    bool LoadTexture(const std::string &id, const std::string &path);
    bool LoadFont(const std::string &id, const std::string &path);
    bool LoadShader(const std::string &id, const std::string &vertex_path,
        const std::string &fragment_path);

    Vector2f GetTextureSize(const std::string &id) const;
    FloatRect GetTextBounds(const std::string &text,
        const std::string &font_id, unsigned int character_size) const;

    // ===== Drawing =====

    void DrawSprite(const std::string &texture_id, const Transform &transform,
        const FloatRect *texture_rect = nullptr,
        const Color &color = Color(255, 255, 255, 255),
        const std::string *shader_id = nullptr);

    void DrawText(const std::string &text, const std::string &font_id,
        const Transform &transform, unsigned int character_size,
        const Color &color = Color(255, 255, 255, 255));

    void DrawRectangle(const FloatRect &rect, const Color &color,
        const Color *outline_color = nullptr, float outline_thickness = 0.0f);

    void DrawCircle(const Vector2f &center, float radius, const Color &color,
        const Color *outline_color = nullptr, float outline_thickness = 0.0f);

    // ===== Shader Management =====

    void SetShaderParameter(
        const std::string &shader_id, const std::string &name, float value);

    // ===== Metadata =====

    std::string GetModuleName() const;

    // ===== Direct Access =====

    /**
     * @brief Get raw video module pointer.
     *
     * Use with caution; prefer using adapter methods.
     *
     * @return Raw pointer to video module
     */
    IVideoModule *GetModule() const;

    /**
     * @brief Get underlying SFML RenderWindow for compatibility.
     *
     * This is a temporary bridge method for legacy systems that still
     * use SFML directly. Returns nullptr if backend is not SFML-based.
     *
     * @return Pointer to sf::RenderWindow or nullptr
     */
    void *GetSFMLWindow() const;

 private:
    std::shared_ptr<IVideoModule> module_;
};

}  // namespace Video
}  // namespace Engine
