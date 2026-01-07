/**
 * @file SFMLGraphicsBackend.hpp
 * @brief SFML implementation of the IGraphicsBackend interface.
 *
 * This class wraps SFML rendering operations.
 * It is the ONLY place where SFML graphics rendering should occur.
 *
 * NOTE: This is a MINIMAL implementation for PR 1.7 (plumbing only).
 * Future PRs will expand with texture loading, sprite rendering, etc.
 */

#ifndef CLIENT_GRAPHICS_SFMLGRAPHICSBACKEND_HPP_
#define CLIENT_GRAPHICS_SFMLGRAPHICSBACKEND_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "graphics/IGraphicsBackend.hpp"

namespace Rtype::Client::Graphics {

/**
 * @brief SFML implementation of the graphics backend.
 *
 * Wraps sf::RenderWindow rendering operations.
 * This is the ONLY class allowed to call SFML graphics methods.
 *
 * SCOPE: PR 1.7 - Frame lifecycle only.
 */
class SFMLGraphicsBackend : public Engine::Graphics::IGraphicsBackend {
 public:
    /**
     * @brief Construct an SFML graphics backend.
     *
     * @param window Reference to the SFML render window
     */
    explicit SFMLGraphicsBackend(sf::RenderWindow &window);

    ~SFMLGraphicsBackend() override = default;

    // IGraphicsBackend interface implementation
    void BeginFrame(const Engine::Graphics::Color &clear_color) override;
    void EndFrame() override;

    // TODO(PR 1.8): Add resource loading methods
    // TODO(PR 1.9): Add drawing methods

 private:
    sf::RenderWindow &window_;
};

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_GRAPHICS_SFMLGRAPHICSBACKEND_HPP_
