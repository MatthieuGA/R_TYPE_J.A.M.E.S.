/**
 * @file SFMLEventSource.cpp
 * @brief SFML implementation of IPlatformEventSource.
 */

#include "platform/SFMLEventSource.hpp"

namespace Rtype::Client::Platform {

namespace {

// ============================================================================
// Internal SFML conversion helpers
// ============================================================================

/**
 * @brief Convert SFML keyboard key to Engine key.
 *
 * @param sfml_key The SFML key code
 * @return Engine::Input::Key The corresponding engine key
 */
Engine::Input::Key FromSFMLKey(sf::Keyboard::Key sfml_key) {
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
 * @brief Convert SFML mouse button to Engine mouse button.
 *
 * @param sfml_button The SFML mouse button
 * @return Engine::Input::MouseButton The corresponding engine mouse button
 */
Engine::Input::MouseButton FromSFMLMouseButton(sf::Mouse::Button sfml_button) {
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

}  // anonymous namespace

SFMLEventSource::SFMLEventSource(sf::Window &window) : window_(window) {}

bool SFMLEventSource::Poll(Engine::Platform::OSEvent &out) {
    sf::Event sfml_event;

    // Poll the SFML window for events
    if (!window_.pollEvent(sfml_event)) {
        return false;  // No events available
    }

    // Translate SFML event to OSEvent using existing adapter logic
    switch (sfml_event.type) {
        case sf::Event::Closed:
            out.type = Engine::Platform::OSEventType::Closed;
            return true;

        case sf::Event::Resized:
            out.type = Engine::Platform::OSEventType::Resized;
            out.size.width = sfml_event.size.width;
            out.size.height = sfml_event.size.height;
            return true;

        case sf::Event::LostFocus:
            out.type = Engine::Platform::OSEventType::LostFocus;
            return true;

        case sf::Event::GainedFocus:
            out.type = Engine::Platform::OSEventType::GainedFocus;
            return true;

        case sf::Event::TextEntered:
            out.type = Engine::Platform::OSEventType::TextEntered;
            out.text.unicode = sfml_event.text.unicode;
            return true;

        case sf::Event::KeyPressed:
            out.type = Engine::Platform::OSEventType::KeyPressed;
            out.key.code = FromSFMLKey(sfml_event.key.code);
            out.key.alt = sfml_event.key.alt;
            out.key.control = sfml_event.key.control;
            out.key.shift = sfml_event.key.shift;
            out.key.system = sfml_event.key.system;
            return true;

        case sf::Event::KeyReleased:
            out.type = Engine::Platform::OSEventType::KeyReleased;
            out.key.code = FromSFMLKey(sfml_event.key.code);
            out.key.alt = sfml_event.key.alt;
            out.key.control = sfml_event.key.control;
            out.key.shift = sfml_event.key.shift;
            out.key.system = sfml_event.key.system;
            return true;

        case sf::Event::MouseWheelScrolled:
            out.type = Engine::Platform::OSEventType::MouseWheelScrolled;
            out.mouseWheel.delta = sfml_event.mouseWheelScroll.delta;
            out.mouseWheel.x = sfml_event.mouseWheelScroll.x;
            out.mouseWheel.y = sfml_event.mouseWheelScroll.y;
            return true;

        case sf::Event::MouseButtonPressed:
            out.type = Engine::Platform::OSEventType::MouseButtonPressed;
            out.mouseButton.button =
                FromSFMLMouseButton(sfml_event.mouseButton.button);
            out.mouseButton.x = sfml_event.mouseButton.x;
            out.mouseButton.y = sfml_event.mouseButton.y;
            return true;

        case sf::Event::MouseButtonReleased:
            out.type = Engine::Platform::OSEventType::MouseButtonReleased;
            out.mouseButton.button =
                FromSFMLMouseButton(sfml_event.mouseButton.button);
            out.mouseButton.x = sfml_event.mouseButton.x;
            out.mouseButton.y = sfml_event.mouseButton.y;
            return true;

        case sf::Event::MouseMoved:
            out.type = Engine::Platform::OSEventType::MouseMoved;
            out.mouseMove.x = sfml_event.mouseMove.x;
            out.mouseMove.y = sfml_event.mouseMove.y;
            return true;

        case sf::Event::MouseEntered:
            out.type = Engine::Platform::OSEventType::MouseEntered;
            return true;

        case sf::Event::MouseLeft:
            out.type = Engine::Platform::OSEventType::MouseLeft;
            return true;

        default:
            // Unsupported event type (joystick, touch, etc.) - skip it
            // Recursively poll for the next event
            return Poll(out);
    }
}

}  // namespace Rtype::Client::Platform
