/**
 * @file SFMLInputUtils.hpp
 * @brief SFML-specific input utility functions.
 *
 * Provides convenience functions for querying input state using SFML.
 * These utilities convert Engine::Input types to SFML types for state queries.
 */

#ifndef CLIENT_INPUT_UTILS_SFMLINPUTUTILS_HPP_
#define CLIENT_INPUT_UTILS_SFMLINPUTUTILS_HPP_

#include <SFML/Window.hpp>

#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Rtype::Client::Input {

/**
 * @brief Check if an engine key is pressed using SFML.
 *
 * Provides a backend-agnostic way to query keyboard state by mapping
 * Engine::Input::Key to the underlying SFML key representation.
 *
 * @param key The engine key to check
 * @return bool True if the key is pressed, false otherwise
 */
inline bool IsKeyPressed(Engine::Input::Key key) {
    // Map engine key to SFML key for state query
    sf::Keyboard::Key sfml_key;
    switch (key) {
        case Engine::Input::Key::A:
            sfml_key = sf::Keyboard::A;
            break;
        case Engine::Input::Key::B:
            sfml_key = sf::Keyboard::B;
            break;
        case Engine::Input::Key::C:
            sfml_key = sf::Keyboard::C;
            break;
        case Engine::Input::Key::D:
            sfml_key = sf::Keyboard::D;
            break;
        case Engine::Input::Key::E:
            sfml_key = sf::Keyboard::E;
            break;
        case Engine::Input::Key::F:
            sfml_key = sf::Keyboard::F;
            break;
        case Engine::Input::Key::G:
            sfml_key = sf::Keyboard::G;
            break;
        case Engine::Input::Key::H:
            sfml_key = sf::Keyboard::H;
            break;
        case Engine::Input::Key::I:
            sfml_key = sf::Keyboard::I;
            break;
        case Engine::Input::Key::J:
            sfml_key = sf::Keyboard::J;
            break;
        case Engine::Input::Key::K:
            sfml_key = sf::Keyboard::K;
            break;
        case Engine::Input::Key::L:
            sfml_key = sf::Keyboard::L;
            break;
        case Engine::Input::Key::M:
            sfml_key = sf::Keyboard::M;
            break;
        case Engine::Input::Key::N:
            sfml_key = sf::Keyboard::N;
            break;
        case Engine::Input::Key::O:
            sfml_key = sf::Keyboard::O;
            break;
        case Engine::Input::Key::P:
            sfml_key = sf::Keyboard::P;
            break;
        case Engine::Input::Key::Q:
            sfml_key = sf::Keyboard::Q;
            break;
        case Engine::Input::Key::R:
            sfml_key = sf::Keyboard::R;
            break;
        case Engine::Input::Key::S:
            sfml_key = sf::Keyboard::S;
            break;
        case Engine::Input::Key::T:
            sfml_key = sf::Keyboard::T;
            break;
        case Engine::Input::Key::U:
            sfml_key = sf::Keyboard::U;
            break;
        case Engine::Input::Key::V:
            sfml_key = sf::Keyboard::V;
            break;
        case Engine::Input::Key::W:
            sfml_key = sf::Keyboard::W;
            break;
        case Engine::Input::Key::X:
            sfml_key = sf::Keyboard::X;
            break;
        case Engine::Input::Key::Y:
            sfml_key = sf::Keyboard::Y;
            break;
        case Engine::Input::Key::Z:
            sfml_key = sf::Keyboard::Z;
            break;
        case Engine::Input::Key::Num0:
            sfml_key = sf::Keyboard::Num0;
            break;
        case Engine::Input::Key::Num1:
            sfml_key = sf::Keyboard::Num1;
            break;
        case Engine::Input::Key::Num2:
            sfml_key = sf::Keyboard::Num2;
            break;
        case Engine::Input::Key::Num3:
            sfml_key = sf::Keyboard::Num3;
            break;
        case Engine::Input::Key::Num4:
            sfml_key = sf::Keyboard::Num4;
            break;
        case Engine::Input::Key::Num5:
            sfml_key = sf::Keyboard::Num5;
            break;
        case Engine::Input::Key::Num6:
            sfml_key = sf::Keyboard::Num6;
            break;
        case Engine::Input::Key::Num7:
            sfml_key = sf::Keyboard::Num7;
            break;
        case Engine::Input::Key::Num8:
            sfml_key = sf::Keyboard::Num8;
            break;
        case Engine::Input::Key::Num9:
            sfml_key = sf::Keyboard::Num9;
            break;
        case Engine::Input::Key::Left:
            sfml_key = sf::Keyboard::Left;
            break;
        case Engine::Input::Key::Right:
            sfml_key = sf::Keyboard::Right;
            break;
        case Engine::Input::Key::Up:
            sfml_key = sf::Keyboard::Up;
            break;
        case Engine::Input::Key::Down:
            sfml_key = sf::Keyboard::Down;
            break;
        case Engine::Input::Key::Space:
            sfml_key = sf::Keyboard::Space;
            break;
        case Engine::Input::Key::Enter:
            sfml_key = sf::Keyboard::Enter;
            break;
        case Engine::Input::Key::Escape:
            sfml_key = sf::Keyboard::Escape;
            break;
        case Engine::Input::Key::Tab:
            sfml_key = sf::Keyboard::Tab;
            break;
        case Engine::Input::Key::Backspace:
            sfml_key = sf::Keyboard::Backspace;
            break;
        case Engine::Input::Key::Delete:
            sfml_key = sf::Keyboard::Delete;
            break;
        case Engine::Input::Key::F1:
            sfml_key = sf::Keyboard::F1;
            break;
        case Engine::Input::Key::F2:
            sfml_key = sf::Keyboard::F2;
            break;
        case Engine::Input::Key::F3:
            sfml_key = sf::Keyboard::F3;
            break;
        case Engine::Input::Key::F4:
            sfml_key = sf::Keyboard::F4;
            break;
        case Engine::Input::Key::F5:
            sfml_key = sf::Keyboard::F5;
            break;
        case Engine::Input::Key::F6:
            sfml_key = sf::Keyboard::F6;
            break;
        case Engine::Input::Key::F7:
            sfml_key = sf::Keyboard::F7;
            break;
        case Engine::Input::Key::F8:
            sfml_key = sf::Keyboard::F8;
            break;
        case Engine::Input::Key::F9:
            sfml_key = sf::Keyboard::F9;
            break;
        case Engine::Input::Key::F10:
            sfml_key = sf::Keyboard::F10;
            break;
        case Engine::Input::Key::F11:
            sfml_key = sf::Keyboard::F11;
            break;
        case Engine::Input::Key::F12:
            sfml_key = sf::Keyboard::F12;
            break;
        case Engine::Input::Key::LShift:
            sfml_key = sf::Keyboard::LShift;
            break;
        case Engine::Input::Key::RShift:
            sfml_key = sf::Keyboard::RShift;
            break;
        case Engine::Input::Key::LControl:
            sfml_key = sf::Keyboard::LControl;
            break;
        case Engine::Input::Key::RControl:
            sfml_key = sf::Keyboard::RControl;
            break;
        case Engine::Input::Key::LAlt:
            sfml_key = sf::Keyboard::LAlt;
            break;
        case Engine::Input::Key::RAlt:
            sfml_key = sf::Keyboard::RAlt;
            break;
        default:
            return false;  // Unknown key is never pressed
    }
    return sf::Keyboard::isKeyPressed(sfml_key);
}

/**
 * @brief Check if an engine mouse button is pressed using SFML.
 *
 * Provides a backend-agnostic way to query mouse button state by mapping
 * Engine::Input::MouseButton to the underlying SFML button representation.
 *
 * @param button The engine mouse button to check
 * @return bool True if the button is pressed, false otherwise
 */
inline bool IsMouseButtonPressed(Engine::Input::MouseButton button) {
    sf::Mouse::Button sfml_button;
    switch (button) {
        case Engine::Input::MouseButton::Left:
            sfml_button = sf::Mouse::Left;
            break;
        case Engine::Input::MouseButton::Right:
            sfml_button = sf::Mouse::Right;
            break;
        case Engine::Input::MouseButton::Middle:
            sfml_button = sf::Mouse::Middle;
            break;
        case Engine::Input::MouseButton::XButton1:
            sfml_button = sf::Mouse::XButton1;
            break;
        case Engine::Input::MouseButton::XButton2:
            sfml_button = sf::Mouse::XButton2;
            break;
        default:
            return false;
    }
    return sf::Mouse::isButtonPressed(sfml_button);
}

}  // namespace Rtype::Client::Input

#endif  // CLIENT_INPUT_UTILS_SFMLINPUTUTILS_HPP_
