/**
 * @file SFMLEventSource.cpp
 * @brief SFML implementation of IPlatformEventSource.
 */

#include "platform/SFMLEventSource.hpp"

#include "adapters/SFMLInputAdapters.hpp"

namespace Rtype::Client::Platform {

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
            out.key.code = Adapters::FromSFMLKey(sfml_event.key.code);
            out.key.alt = sfml_event.key.alt;
            out.key.control = sfml_event.key.control;
            out.key.shift = sfml_event.key.shift;
            out.key.system = sfml_event.key.system;
            return true;

        case sf::Event::KeyReleased:
            out.type = Engine::Platform::OSEventType::KeyReleased;
            out.key.code = Adapters::FromSFMLKey(sfml_event.key.code);
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
                Adapters::FromSFMLMouseButton(sfml_event.mouseButton.button);
            out.mouseButton.x = sfml_event.mouseButton.x;
            out.mouseButton.y = sfml_event.mouseButton.y;
            return true;

        case sf::Event::MouseButtonReleased:
            out.type = Engine::Platform::OSEventType::MouseButtonReleased;
            out.mouseButton.button =
                Adapters::FromSFMLMouseButton(sfml_event.mouseButton.button);
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
