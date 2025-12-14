/**
 * @file TypeConversions.hpp
 * @brief Utility functions to convert between SFML and Engine types.
 *
 * This header provides conversion functions to bridge legacy SFML-based
 * components with the new rendering engine abstraction layer.
 */

#pragma once
#include <SFML/Graphics.hpp>

#include "graphics/Types.hpp"

namespace Rtype::Client {

/**
 * @brief Convert sf::Vector2f to Engine::Graphics::Vector2f
 */
inline Engine::Graphics::Vector2f ToEngineVector(const sf::Vector2f &v) {
    return Engine::Graphics::Vector2f(v.x, v.y);
}

/**
 * @brief Convert Engine::Graphics::Vector2f to sf::Vector2f
 */
inline sf::Vector2f ToSFMLVector(const Engine::Graphics::Vector2f &v) {
    return sf::Vector2f(v.x, v.y);
}

/**
 * @brief Convert sf::Color to Engine::Graphics::Color
 */
inline Engine::Graphics::Color ToEngineColor(const sf::Color &c) {
    return Engine::Graphics::Color(c.r, c.g, c.b, c.a);
}

/**
 * @brief Convert Engine::Graphics::Color to sf::Color
 */
inline sf::Color ToSFMLColor(const Engine::Graphics::Color &c) {
    return sf::Color(c.r, c.g, c.b, c.a);
}

/**
 * @brief Convert sf::IntRect to Engine::Graphics::IntRect
 */
inline Engine::Graphics::IntRect ToEngineRect(const sf::IntRect &r) {
    return Engine::Graphics::IntRect(r.left, r.top, r.width, r.height);
}

/**
 * @brief Convert Engine::Graphics::IntRect to sf::IntRect
 */
inline sf::IntRect ToSFMLRect(const Engine::Graphics::IntRect &r) {
    return sf::IntRect(r.left, r.top, r.width, r.height);
}

}  // namespace Rtype::Client
