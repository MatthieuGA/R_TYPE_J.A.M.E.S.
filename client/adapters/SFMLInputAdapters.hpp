/**
 * @file SFMLInputAdapters.hpp
 * @brief Adapters to convert between SFML and Engine::Input types.
 *
 * Provides conversion functions between SFML input types (sf::Keyboard::Key,
 * sf::Mouse::Button, sf::Event) and Engine input types (Engine::Input::Key,
 * Engine::Input::MouseButton, Engine::Input::Event).
 *
 * These adapters exist ONLY at the SFML boundary to convert external library
 * types to our engine-agnostic abstractions.
 */

#ifndef CLIENT_ADAPTERS_SFMLINPUTADAPTERS_HPP_
#define CLIENT_ADAPTERS_SFMLINPUTADAPTERS_HPP_

#include <SFML/Window.hpp>

#include "input/Event.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Rtype::Client::Adapters {

/**
 * @brief Convert SFML keyboard key to Engine key.
 *
 * @param sfml_key The SFML key code
 * @return Engine::Input::Key The corresponding engine key
 */
inline Engine::Input::Key FromSFMLKey(sf::Keyboard::Key sfml_key) {
    switch (sfml_key) {
        case sf::Keyboard::A:
            return Engine::Input::Key::A;
        case sf::Keyboard::B:
            return Engine::Input::Key::B;
        case sf::Keyboard::C:
            return Engine::Input::Key::C;
        case sf::Keyboard::D:
            return Engine::Input::Key::D;
        case sf::Keyboard::E:
            return Engine::Input::Key::E;
        case sf::Keyboard::F:
            return Engine::Input::Key::F;
        case sf::Keyboard::G:
            return Engine::Input::Key::G;
        case sf::Keyboard::H:
            return Engine::Input::Key::H;
        case sf::Keyboard::I:
            return Engine::Input::Key::I;
        case sf::Keyboard::J:
            return Engine::Input::Key::J;
        case sf::Keyboard::K:
            return Engine::Input::Key::K;
        case sf::Keyboard::L:
            return Engine::Input::Key::L;
        case sf::Keyboard::M:
            return Engine::Input::Key::M;
        case sf::Keyboard::N:
            return Engine::Input::Key::N;
        case sf::Keyboard::O:
            return Engine::Input::Key::O;
        case sf::Keyboard::P:
            return Engine::Input::Key::P;
        case sf::Keyboard::Q:
            return Engine::Input::Key::Q;
        case sf::Keyboard::R:
            return Engine::Input::Key::R;
        case sf::Keyboard::S:
            return Engine::Input::Key::S;
        case sf::Keyboard::T:
            return Engine::Input::Key::T;
        case sf::Keyboard::U:
            return Engine::Input::Key::U;
        case sf::Keyboard::V:
            return Engine::Input::Key::V;
        case sf::Keyboard::W:
            return Engine::Input::Key::W;
        case sf::Keyboard::X:
            return Engine::Input::Key::X;
        case sf::Keyboard::Y:
            return Engine::Input::Key::Y;
        case sf::Keyboard::Z:
            return Engine::Input::Key::Z;
        case sf::Keyboard::Num0:
            return Engine::Input::Key::Num0;
        case sf::Keyboard::Num1:
            return Engine::Input::Key::Num1;
        case sf::Keyboard::Num2:
            return Engine::Input::Key::Num2;
        case sf::Keyboard::Num3:
            return Engine::Input::Key::Num3;
        case sf::Keyboard::Num4:
            return Engine::Input::Key::Num4;
        case sf::Keyboard::Num5:
            return Engine::Input::Key::Num5;
        case sf::Keyboard::Num6:
            return Engine::Input::Key::Num6;
        case sf::Keyboard::Num7:
            return Engine::Input::Key::Num7;
        case sf::Keyboard::Num8:
            return Engine::Input::Key::Num8;
        case sf::Keyboard::Num9:
            return Engine::Input::Key::Num9;
        case sf::Keyboard::Left:
            return Engine::Input::Key::Left;
        case sf::Keyboard::Right:
            return Engine::Input::Key::Right;
        case sf::Keyboard::Up:
            return Engine::Input::Key::Up;
        case sf::Keyboard::Down:
            return Engine::Input::Key::Down;
        case sf::Keyboard::Space:
            return Engine::Input::Key::Space;
        case sf::Keyboard::Enter:
            return Engine::Input::Key::Enter;
        case sf::Keyboard::Escape:
            return Engine::Input::Key::Escape;
        case sf::Keyboard::Tab:
            return Engine::Input::Key::Tab;
        case sf::Keyboard::Backspace:
            return Engine::Input::Key::Backspace;
        case sf::Keyboard::Delete:
            return Engine::Input::Key::Delete;
        case sf::Keyboard::F1:
            return Engine::Input::Key::F1;
        case sf::Keyboard::F2:
            return Engine::Input::Key::F2;
        case sf::Keyboard::F3:
            return Engine::Input::Key::F3;
        case sf::Keyboard::F4:
            return Engine::Input::Key::F4;
        case sf::Keyboard::F5:
            return Engine::Input::Key::F5;
        case sf::Keyboard::F6:
            return Engine::Input::Key::F6;
        case sf::Keyboard::F7:
            return Engine::Input::Key::F7;
        case sf::Keyboard::F8:
            return Engine::Input::Key::F8;
        case sf::Keyboard::F9:
            return Engine::Input::Key::F9;
        case sf::Keyboard::F10:
            return Engine::Input::Key::F10;
        case sf::Keyboard::F11:
            return Engine::Input::Key::F11;
        case sf::Keyboard::F12:
            return Engine::Input::Key::F12;
        case sf::Keyboard::LShift:
            return Engine::Input::Key::LShift;
        case sf::Keyboard::RShift:
            return Engine::Input::Key::RShift;
        case sf::Keyboard::LControl:
            return Engine::Input::Key::LControl;
        case sf::Keyboard::RControl:
            return Engine::Input::Key::RControl;
        case sf::Keyboard::LAlt:
            return Engine::Input::Key::LAlt;
        case sf::Keyboard::RAlt:
            return Engine::Input::Key::RAlt;
        default:
            return Engine::Input::Key::Unknown;
    }
}

/**
 * @brief Convert Engine key to SFML keyboard key.
 *
 * @param engine_key The engine key code
 * @return sf::Keyboard::Key The corresponding SFML key
 */
inline sf::Keyboard::Key ToSFMLKey(Engine::Input::Key engine_key) {
    switch (engine_key) {
        case Engine::Input::Key::A:
            return sf::Keyboard::A;
        case Engine::Input::Key::B:
            return sf::Keyboard::B;
        case Engine::Input::Key::C:
            return sf::Keyboard::C;
        case Engine::Input::Key::D:
            return sf::Keyboard::D;
        case Engine::Input::Key::E:
            return sf::Keyboard::E;
        case Engine::Input::Key::F:
            return sf::Keyboard::F;
        case Engine::Input::Key::G:
            return sf::Keyboard::G;
        case Engine::Input::Key::H:
            return sf::Keyboard::H;
        case Engine::Input::Key::I:
            return sf::Keyboard::I;
        case Engine::Input::Key::J:
            return sf::Keyboard::J;
        case Engine::Input::Key::K:
            return sf::Keyboard::K;
        case Engine::Input::Key::L:
            return sf::Keyboard::L;
        case Engine::Input::Key::M:
            return sf::Keyboard::M;
        case Engine::Input::Key::N:
            return sf::Keyboard::N;
        case Engine::Input::Key::O:
            return sf::Keyboard::O;
        case Engine::Input::Key::P:
            return sf::Keyboard::P;
        case Engine::Input::Key::Q:
            return sf::Keyboard::Q;
        case Engine::Input::Key::R:
            return sf::Keyboard::R;
        case Engine::Input::Key::S:
            return sf::Keyboard::S;
        case Engine::Input::Key::T:
            return sf::Keyboard::T;
        case Engine::Input::Key::U:
            return sf::Keyboard::U;
        case Engine::Input::Key::V:
            return sf::Keyboard::V;
        case Engine::Input::Key::W:
            return sf::Keyboard::W;
        case Engine::Input::Key::X:
            return sf::Keyboard::X;
        case Engine::Input::Key::Y:
            return sf::Keyboard::Y;
        case Engine::Input::Key::Z:
            return sf::Keyboard::Z;
        case Engine::Input::Key::Num0:
            return sf::Keyboard::Num0;
        case Engine::Input::Key::Num1:
            return sf::Keyboard::Num1;
        case Engine::Input::Key::Num2:
            return sf::Keyboard::Num2;
        case Engine::Input::Key::Num3:
            return sf::Keyboard::Num3;
        case Engine::Input::Key::Num4:
            return sf::Keyboard::Num4;
        case Engine::Input::Key::Num5:
            return sf::Keyboard::Num5;
        case Engine::Input::Key::Num6:
            return sf::Keyboard::Num6;
        case Engine::Input::Key::Num7:
            return sf::Keyboard::Num7;
        case Engine::Input::Key::Num8:
            return sf::Keyboard::Num8;
        case Engine::Input::Key::Num9:
            return sf::Keyboard::Num9;
        case Engine::Input::Key::Left:
            return sf::Keyboard::Left;
        case Engine::Input::Key::Right:
            return sf::Keyboard::Right;
        case Engine::Input::Key::Up:
            return sf::Keyboard::Up;
        case Engine::Input::Key::Down:
            return sf::Keyboard::Down;
        case Engine::Input::Key::Space:
            return sf::Keyboard::Space;
        case Engine::Input::Key::Enter:
            return sf::Keyboard::Enter;
        case Engine::Input::Key::Escape:
            return sf::Keyboard::Escape;
        case Engine::Input::Key::Tab:
            return sf::Keyboard::Tab;
        case Engine::Input::Key::Backspace:
            return sf::Keyboard::Backspace;
        case Engine::Input::Key::Delete:
            return sf::Keyboard::Delete;
        case Engine::Input::Key::F1:
            return sf::Keyboard::F1;
        case Engine::Input::Key::F2:
            return sf::Keyboard::F2;
        case Engine::Input::Key::F3:
            return sf::Keyboard::F3;
        case Engine::Input::Key::F4:
            return sf::Keyboard::F4;
        case Engine::Input::Key::F5:
            return sf::Keyboard::F5;
        case Engine::Input::Key::F6:
            return sf::Keyboard::F6;
        case Engine::Input::Key::F7:
            return sf::Keyboard::F7;
        case Engine::Input::Key::F8:
            return sf::Keyboard::F8;
        case Engine::Input::Key::F9:
            return sf::Keyboard::F9;
        case Engine::Input::Key::F10:
            return sf::Keyboard::F10;
        case Engine::Input::Key::F11:
            return sf::Keyboard::F11;
        case Engine::Input::Key::F12:
            return sf::Keyboard::F12;
        case Engine::Input::Key::LShift:
            return sf::Keyboard::LShift;
        case Engine::Input::Key::RShift:
            return sf::Keyboard::RShift;
        case Engine::Input::Key::LControl:
            return sf::Keyboard::LControl;
        case Engine::Input::Key::RControl:
            return sf::Keyboard::RControl;
        case Engine::Input::Key::LAlt:
            return sf::Keyboard::LAlt;
        case Engine::Input::Key::RAlt:
            return sf::Keyboard::RAlt;
        default:
            return sf::Keyboard::Unknown;
    }
}

/**
 * @brief Convert SFML mouse button to Engine mouse button.
 *
 * @param sfml_button The SFML mouse button
 * @return Engine::Input::MouseButton The corresponding engine mouse button
 */
inline Engine::Input::MouseButton FromSFMLMouseButton(
    sf::Mouse::Button sfml_button) {
    switch (sfml_button) {
        case sf::Mouse::Left:
            return Engine::Input::MouseButton::Left;
        case sf::Mouse::Right:
            return Engine::Input::MouseButton::Right;
        case sf::Mouse::Middle:
            return Engine::Input::MouseButton::Middle;
        case sf::Mouse::XButton1:
            return Engine::Input::MouseButton::XButton1;
        case sf::Mouse::XButton2:
            return Engine::Input::MouseButton::XButton2;
        default:
            return Engine::Input::MouseButton::Left;
    }
}

/**
 * @brief Convert Engine mouse button to SFML mouse button.
 *
 * @param engine_button The engine mouse button
 * @return sf::Mouse::Button The corresponding SFML mouse button
 */
inline sf::Mouse::Button ToSFMLMouseButton(
    Engine::Input::MouseButton engine_button) {
    switch (engine_button) {
        case Engine::Input::MouseButton::Left:
            return sf::Mouse::Left;
        case Engine::Input::MouseButton::Right:
            return sf::Mouse::Right;
        case Engine::Input::MouseButton::Middle:
            return sf::Mouse::Middle;
        case Engine::Input::MouseButton::XButton1:
            return sf::Mouse::XButton1;
        case Engine::Input::MouseButton::XButton2:
            return sf::Mouse::XButton2;
        default:
            return sf::Mouse::Left;
    }
}

/**
 * @brief Check if an engine key is pressed using SFML.
 *
 * @param key The engine key to check
 * @return bool True if the key is pressed, false otherwise
 */
inline bool IsKeyPressed(Engine::Input::Key key) {
    return sf::Keyboard::isKeyPressed(ToSFMLKey(key));
}

/**
 * @brief Check if an engine mouse button is pressed using SFML.
 *
 * @param button The engine mouse button to check
 * @return bool True if the button is pressed, false otherwise
 */
inline bool IsMouseButtonPressed(Engine::Input::MouseButton button) {
    return sf::Mouse::isButtonPressed(ToSFMLMouseButton(button));
}

/**
 * @brief Convert SFML event to Engine event.
 *
 * @param sfml_event The SFML event to convert
 * @param engine_event The engine event to populate
 * @return bool True if conversion succeeded, false otherwise
 */
inline bool FromSFMLEvent(
    const sf::Event &sfml_event, Engine::Input::Event &engine_event) {
    switch (sfml_event.type) {
        case sf::Event::Closed:
            engine_event.type = Engine::Input::EventType::Closed;
            return true;

        case sf::Event::Resized:
            engine_event.type = Engine::Input::EventType::Resized;
            engine_event.size.width = sfml_event.size.width;
            engine_event.size.height = sfml_event.size.height;
            return true;

        case sf::Event::LostFocus:
            engine_event.type = Engine::Input::EventType::LostFocus;
            return true;

        case sf::Event::GainedFocus:
            engine_event.type = Engine::Input::EventType::GainedFocus;
            return true;

        case sf::Event::TextEntered:
            engine_event.type = Engine::Input::EventType::TextEntered;
            engine_event.text.unicode = sfml_event.text.unicode;
            return true;

        case sf::Event::KeyPressed:
            engine_event.type = Engine::Input::EventType::KeyPressed;
            engine_event.key.code = FromSFMLKey(sfml_event.key.code);
            engine_event.key.alt = sfml_event.key.alt;
            engine_event.key.control = sfml_event.key.control;
            engine_event.key.shift = sfml_event.key.shift;
            engine_event.key.system = sfml_event.key.system;
            return true;

        case sf::Event::KeyReleased:
            engine_event.type = Engine::Input::EventType::KeyReleased;
            engine_event.key.code = FromSFMLKey(sfml_event.key.code);
            engine_event.key.alt = sfml_event.key.alt;
            engine_event.key.control = sfml_event.key.control;
            engine_event.key.shift = sfml_event.key.shift;
            engine_event.key.system = sfml_event.key.system;
            return true;

        case sf::Event::MouseWheelScrolled:
            engine_event.type = Engine::Input::EventType::MouseWheelScrolled;
            engine_event.mouseWheel.delta = sfml_event.mouseWheelScroll.delta;
            engine_event.mouseWheel.x = sfml_event.mouseWheelScroll.x;
            engine_event.mouseWheel.y = sfml_event.mouseWheelScroll.y;
            return true;

        case sf::Event::MouseButtonPressed:
            engine_event.type = Engine::Input::EventType::MouseButtonPressed;
            engine_event.mouseButton.button =
                FromSFMLMouseButton(sfml_event.mouseButton.button);
            engine_event.mouseButton.x = sfml_event.mouseButton.x;
            engine_event.mouseButton.y = sfml_event.mouseButton.y;
            return true;

        case sf::Event::MouseButtonReleased:
            engine_event.type = Engine::Input::EventType::MouseButtonReleased;
            engine_event.mouseButton.button =
                FromSFMLMouseButton(sfml_event.mouseButton.button);
            engine_event.mouseButton.x = sfml_event.mouseButton.x;
            engine_event.mouseButton.y = sfml_event.mouseButton.y;
            return true;

        case sf::Event::MouseMoved:
            engine_event.type = Engine::Input::EventType::MouseMoved;
            engine_event.mouseMove.x = sfml_event.mouseMove.x;
            engine_event.mouseMove.y = sfml_event.mouseMove.y;
            return true;

        case sf::Event::MouseEntered:
            engine_event.type = Engine::Input::EventType::MouseEntered;
            return true;

        case sf::Event::MouseLeft:
            engine_event.type = Engine::Input::EventType::MouseLeft;
            return true;

        default:
            // Unsupported event type (joystick, touch, etc.)
            return false;
    }
}

}  // namespace Rtype::Client::Adapters

#endif  // CLIENT_ADAPTERS_SFMLINPUTADAPTERS_HPP_
