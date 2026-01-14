/**
 * @file SFMLGraphicsUtils.hpp
 * @brief SFML-specific graphics utility functions.
 *
 * Provides convenience functions for converting between Engine graphics types
 * and SFML graphics types. These utilities will be used by the future
 * SFMLGraphicsBackend implementation.
 *
 * NOTE: This file is temporary until full graphics backend abstraction
 * is implemented. It will be merged into SFMLGraphicsBackend when ready.
 */

#ifndef CLIENT_GRAPHICS_UTILS_SFMLGRAPHICSUTILS_HPP_
#define CLIENT_GRAPHICS_UTILS_SFMLGRAPHICSUTILS_HPP_

#include <SFML/Graphics/Color.hpp>

#include "graphics/Types.hpp"

namespace Rtype::Client::Graphics {

/**
 * @brief Convert Engine color to SFML color.
 *
 * @param engine_color The engine color
 * @return sf::Color The corresponding SFML color
 */
inline sf::Color ToSFMLColor(const Engine::Graphics::Color &engine_color) {
    return sf::Color(
        engine_color.r, engine_color.g, engine_color.b, engine_color.a);
}

/**
 * @brief Convert SFML color to Engine color.
 *
 * @param sfml_color The SFML color
 * @return Engine::Graphics::Color The corresponding engine color
 */
inline Engine::Graphics::Color FromSFMLColor(const sf::Color &sfml_color) {
    return Engine::Graphics::Color(
        sfml_color.r, sfml_color.g, sfml_color.b, sfml_color.a);
}

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_GRAPHICS_UTILS_SFMLGRAPHICSUTILS_HPP_
