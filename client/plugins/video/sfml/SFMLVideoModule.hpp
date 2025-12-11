/**
 * @file SFMLVideoModule.hpp
 * @brief SFML implementation of IVideoModule interface.
 */

#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "video/IVideoModule.hpp"

/**
 * @brief SFML-based video backend implementation.
 */
class SFMLVideoModule : public Engine::Video::IVideoModule {
 public:
    SFMLVideoModule() = default;
    ~SFMLVideoModule() override = default;

    // Lifecycle
    bool Initialize(unsigned int width, unsigned int height,
        const std::string &title) override;
    void Shutdown() override;
    void Update(float delta_time) override;

    // Window Management
    bool IsWindowOpen() const override;
    void CloseWindow() override;
    Engine::Video::Vector2f GetWindowSize() const override;
    void SetWindowTitle(const std::string &title) override;

    // Event Handling
    bool PollEvent(Engine::Video::Event &event) override;

    // Rendering
    void Clear(const Engine::Video::Color &color) override;
    void Display() override;

    // Texture Management
    bool LoadTexture(const std::string &id, const std::string &path) override;
    const void *GetTexture(const std::string &id) const override;
    Engine::Video::Vector2f GetTextureSize(
        const std::string &id) const override;

    // Font Management
    bool LoadFont(const std::string &id, const std::string &path) override;
    const void *GetFont(const std::string &id) const override;
    Engine::Video::FloatRect GetTextBounds(const std::string &text,
        const std::string &font_id,
        unsigned int character_size) const override;

    // Sprite Drawing
    void DrawSprite(const std::string &texture_id,
        const Engine::Video::Transform &transform,
        const Engine::Video::FloatRect *texture_rect,
        const Engine::Video::Color &color,
        const std::string *shader_id) override;

    // Text Drawing
    void DrawText(const std::string &text, const std::string &font_id,
        const Engine::Video::Transform &transform, unsigned int character_size,
        const Engine::Video::Color &color) override;

    // Primitive Drawing
    void DrawRectangle(const Engine::Video::FloatRect &rect,
        const Engine::Video::Color &color,
        const Engine::Video::Color *outline_color,
        float outline_thickness) override;

    void DrawCircle(const Engine::Video::Vector2f &center, float radius,
        const Engine::Video::Color &color,
        const Engine::Video::Color *outline_color,
        float outline_thickness) override;

    // Advanced
    void DrawVertices(const Engine::Video::Vertex *vertices,
        size_t vertex_count, int primitive_type,
        const Engine::Video::RenderStates &states) override;

    // Shader Management
    bool LoadShader(const std::string &id, const std::string &vertex_path,
        const std::string &fragment_path) override;
    void SetShaderParameter(const std::string &shader_id,
        const std::string &name, float value) override;

    // Metadata
    std::string GetModuleName() const override;

    // Compatibility Bridge
    void *GetNativeWindow() const override;

 private:
    std::unique_ptr<sf::RenderWindow> window_;
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts_;
    std::unordered_map<std::string, std::shared_ptr<sf::Shader>> shaders_;

    // Helper conversion functions
    sf::Color ToSFMLColor(const Engine::Video::Color &color) const;
    Engine::Video::EventType FromSFMLEventType(
        sf::Event::EventType type) const;
};

// Plugin entry point (C linkage)
extern "C" {
std::shared_ptr<Engine::Video::IVideoModule> entryPoint();
}
