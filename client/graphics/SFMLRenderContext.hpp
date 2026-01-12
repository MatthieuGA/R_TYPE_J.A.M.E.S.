/**
 * @file SFMLRenderContext.hpp
 * @brief SFML implementation of IRenderContext interface.
 *
 * This class implements the rendering interface using SFML.
 * It is the ONLY place where SFML drawing operations should occur.
 *
 * SCOPE: PR 1.8.1 - Backend implementation ONLY.
 * NO systems, components, or game logic modifications.
 */

#ifndef CLIENT_GRAPHICS_SFMLRENDERCONTEXT_HPP_
#define CLIENT_GRAPHICS_SFMLRENDERCONTEXT_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/RenderWindow.hpp>

#include "graphics/IRenderContext.hpp"

// Forward declarations (SFML types stay in .cpp)
namespace sf {
class Texture;
class Font;
class Shader;
}  // namespace sf

namespace Rtype::Client::Graphics {

/**
 * @brief SFML implementation of the rendering context.
 *
 * Translates engine rendering calls to SFML draw operations.
 * Manages texture and font caching internally.
 *
 * CRITICAL: This preserves exact current rendering behavior.
 * NO enhancements or features beyond what exists today.
 */
class SFMLRenderContext : public Engine::Graphics::IRenderContext {
 public:
    /**
     * @brief Construct an SFML render context.
     *
     * @param window Reference to the SFML render window
     */
    explicit SFMLRenderContext(sf::RenderWindow &window);

    ~SFMLRenderContext() override = default;

    // IRenderContext interface implementation
    void DrawSprite(const Engine::Graphics::DrawableSprite &sprite,
        const Engine::Graphics::DrawableShader *shader = nullptr) override;

    void DrawText(const Engine::Graphics::DrawableText &text) override;

    void DrawRectangle(
        const Engine::Graphics::DrawableRectangle &rect) override;

    void DrawVertexArray(
        const Engine::Graphics::VertexArray &vertices) override;

 private:
    /**
     * @brief Get or load a texture from cache.
     *
     * @param path Path to texture file
     * @return Pointer to texture, or nullptr if failed to load
     */
    sf::Texture *GetOrLoadTexture(const char *path);

    /**
     * @brief Get or load a font from cache.
     *
     * @param path Path to font file
     * @return Pointer to font, or nullptr if failed to load
     */
    sf::Font *GetOrLoadFont(const char *path);

    /**
     * @brief Convert engine color to SFML color.
     *
     * @param color Engine color
     * @return SFML color
     */
    sf::Color ToSFMLColor(const Engine::Graphics::Color &color);

    /// Get or load a shader from cache.
    sf::Shader *GetOrLoadShader(
        const Engine::Graphics::DrawableShader &shader);

    /// Reference to SFML window
    sf::RenderWindow &window_;

    /// Texture cache (path -> texture)
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>>
        texture_cache_;

    /// Font cache (path -> font)
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> font_cache_;

    /// Shader cache (path -> shader)
    std::unordered_map<std::string, std::shared_ptr<sf::Shader>> shader_cache_;
};

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_GRAPHICS_SFMLRENDERCONTEXT_HPP_
