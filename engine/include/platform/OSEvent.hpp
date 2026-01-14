/**
 * @file OSEvent.hpp
 * @brief Backend-agnostic OS/platform event types for the Engine.
 *
 * Provides an engine-agnostic representation of operating system and window
 * events. These are raw platform messages (close, resize, focus, key/mouse
 * input) independent of the underlying backend (SFML, SDL, etc.).
 *
 * NOTE: This is separate from engine gameplay events (ECS event system).
 * OSEvents represent low-level platform input/window messages.
 */

#ifndef ENGINE_INCLUDE_PLATFORM_OSEVENT_HPP_
#define ENGINE_INCLUDE_PLATFORM_OSEVENT_HPP_

#include <cstdint>

#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Engine {
namespace Platform {

/**
 * @brief Enumeration of OS event types.
 *
 * Represents different types of window and input events from the OS/platform.
 */
enum class OSEventType {
    Closed = 0,              ///< The window requested to be closed
    Resized,                 ///< The window was resized
    LostFocus,               ///< The window lost focus
    GainedFocus,             ///< The window gained focus
    TextEntered,             ///< A character was entered
    KeyPressed,              ///< A key was pressed
    KeyReleased,             ///< A key was released
    MouseWheelScrolled,      ///< The mouse wheel was scrolled
    MouseButtonPressed,      ///< A mouse button was pressed
    MouseButtonReleased,     ///< A mouse button was released
    MouseMoved,              ///< The mouse cursor moved
    MouseEntered,            ///< The mouse cursor entered the window
    MouseLeft,               ///< The mouse cursor left the window
    JoystickButtonPressed,   ///< A joystick button was pressed
    JoystickButtonReleased,  ///< A joystick button was released
    JoystickMoved,           ///< A joystick axis moved
    JoystickConnected,       ///< A joystick was connected
    JoystickDisconnected,    ///< A joystick was disconnected
    Count                    ///< Keep last -- the total number of event types
};

/**
 * @brief Keyboard event data.
 *
 * Contains information about a keyboard event.
 */
struct KeyEvent {
    Input::Key code;  ///< Code of the key that has been pressed
    bool alt;         ///< Is the Alt key pressed?
    bool control;     ///< Is the Control key pressed?
    bool shift;       ///< Is the Shift key pressed?
    bool system;      ///< Is the System key pressed?
};

/**
 * @brief Mouse button event data.
 *
 * Contains information about a mouse button event.
 */
struct MouseButtonEvent {
    Input::MouseButton button;  ///< Code of the button that has been pressed
    int x;  ///< X position of the mouse pointer, relative to the left of the
            ///< window
    int y;  ///< Y position of the mouse pointer, relative to the top of the
            ///< window
};

/**
 * @brief Mouse move event data.
 *
 * Contains information about a mouse move event.
 */
struct MouseMoveEvent {
    int x;  ///< X position of the mouse pointer, relative to the left of the
            ///< window
    int y;  ///< Y position of the mouse pointer, relative to the top of the
            ///< window
};

/**
 * @brief Mouse wheel scroll event data.
 *
 * Contains information about a mouse wheel scroll event.
 */
struct MouseWheelScrollEvent {
    float delta;  ///< Wheel offset (positive is up, negative is down)
    int x;  ///< X position of the mouse pointer, relative to the left of the
            ///< window
    int y;  ///< Y position of the mouse pointer, relative to the top of the
            ///< window
};

/**
 * @brief Size event data.
 *
 * Contains information about a resize event.
 */
struct SizeEvent {
    unsigned int width;   ///< New width, in pixels
    unsigned int height;  ///< New height, in pixels
};

/**
 * @brief Text event data.
 *
 * Contains information about a text entered event.
 */
struct TextEvent {
    uint32_t unicode;  ///< UTF-32 Unicode value of the character
};

/**
 * @brief Generic OS event structure.
 *
 * Encapsulates all types of OS/platform events that can occur.
 * The type field indicates which union member is valid.
 */
struct OSEvent {
    OSEventType type;  ///< Type of the event

    union {
        SizeEvent size;                    ///< Size event parameters
        KeyEvent key;                      ///< Key event parameters
        TextEvent text;                    ///< Text event parameters
        MouseMoveEvent mouseMove;          ///< Mouse move event parameters
        MouseButtonEvent mouseButton;      ///< Mouse button event parameters
        MouseWheelScrollEvent mouseWheel;  ///< Mouse wheel event parameters
    };
};

}  // namespace Platform
}  // namespace Engine

#endif  // ENGINE_INCLUDE_PLATFORM_OSEVENT_HPP_
