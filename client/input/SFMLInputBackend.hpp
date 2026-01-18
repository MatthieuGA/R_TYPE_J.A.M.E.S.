/**
 * @file SFMLInputBackend.hpp
 * @brief SFML implementation of the IInputBackend interface.
 *
 * This backend uses SFML for raw input state queries. It wraps sf::Keyboard,
 * sf::Mouse, and sf::Window calls to provide input state to the InputManager.
 */

#ifndef CLIENT_INPUT_SFMLINPUTBACKEND_HPP_
#define CLIENT_INPUT_SFMLINPUTBACKEND_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "input/IInputBackend.hpp"

namespace Rtype::Client::Input {

/**
 * @brief SFML-based input backend implementation.
 *
 * Implements the IInputBackend interface using SFML for raw input queries.
 * Requires a reference to the window for focus and mouse position queries.
 */
class SFMLInputBackend : public Engine::Input::IInputBackend {
 public:
    /**
     * @brief Construct with a reference to the render window.
     *
     * @param window Reference to the SFML render window.
     *               Must remain valid for the lifetime of this backend.
     */
    explicit SFMLInputBackend(sf::RenderWindow &window);

    ~SFMLInputBackend() override = default;

    // IInputBackend implementation
    bool IsKeyPressed(Engine::Input::Key key) const override;
    bool IsMouseButtonPressed(
        Engine::Input::MouseButton button) const override;
    Engine::Input::MousePosition GetMousePosition() const override;
    Engine::Input::MousePosition GetMousePositionInWindow() const override;
    bool HasWindowFocus() const override;

 private:
    sf::RenderWindow &window_;
};

}  // namespace Rtype::Client::Input

#endif  // CLIENT_INPUT_SFMLINPUTBACKEND_HPP_
