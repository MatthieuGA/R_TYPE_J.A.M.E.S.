/**
 * @file SFMLInputBackend.cpp
 * @brief Implementation of the SFMLInputBackend class.
 */

#include "input/SFMLInputBackend.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "input/utils/SFMLInputUtils.hpp"

namespace Rtype::Client::Input {

SFMLInputBackend::SFMLInputBackend(sf::RenderWindow &window)
    : window_(window) {}

bool SFMLInputBackend::IsKeyPressed(Engine::Input::Key key) const {
    return Input::IsKeyPressed(key);
}

bool SFMLInputBackend::IsMouseButtonPressed(
    Engine::Input::MouseButton button) const {
    return Input::IsMouseButtonPressed(button);
}

Engine::Input::MousePosition SFMLInputBackend::GetMousePosition() const {
    sf::Vector2i pos = sf::Mouse::getPosition();
    return {pos.x, pos.y};
}

Engine::Input::MousePosition SFMLInputBackend::GetMousePositionInWindow()
    const {
    sf::Vector2i pos = sf::Mouse::getPosition(window_);
    return {pos.x, pos.y};
}

bool SFMLInputBackend::HasWindowFocus() const {
    return window_.hasFocus();
}

}  // namespace Rtype::Client::Input
