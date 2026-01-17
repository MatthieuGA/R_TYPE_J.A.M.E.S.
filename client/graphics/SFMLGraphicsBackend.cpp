/**
 * @file SFMLGraphicsBackend.cpp
 * @brief Implementation of SFMLGraphicsBackend.
 */

#include "graphics/SFMLGraphicsBackend.hpp"

#include <SFML/Graphics/Color.hpp>

namespace Rtype::Client::Graphics {

SFMLGraphicsBackend::SFMLGraphicsBackend(sf::RenderWindow &window)
    : window_(window) {}

void SFMLGraphicsBackend::BeginFrame(
    const Engine::Graphics::Color &clear_color) {
    // Convert Engine::Graphics::Color to sf::Color
    sf::Color sfml_color(
        clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    window_.clear(sfml_color);
}

void SFMLGraphicsBackend::EndFrame() {
    // Note: window_.display() is called by the window itself,
    // not by the graphics backend. This keeps window lifecycle
    // separate from rendering operations.
}

}  // namespace Rtype::Client::Graphics
