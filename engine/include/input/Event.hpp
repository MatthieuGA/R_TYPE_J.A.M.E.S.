/**
 * @file Event.hpp
 * @brief Backend-agnostic event types for the Engine.
 *
 * Provides an engine-agnostic representation of input events.
 * This allows the client to handle events independent of the
 * underlying input library (e.g., SFML).
 */

#ifndef ENGINE_INCLUDE_INPUT_EVENT_HPP_
#define ENGINE_INCLUDE_INPUT_EVENT_HPP_

#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Engine {
namespace Input {

/**
 * @brief Enumeration of event types.
 *
 * Represents different types of window and input events.
 */
enum class EventType {
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
    Key code;      ///< Code of the key that has been pressed
    bool alt;      ///< Is the Alt key pressed?
    bool control;  ///< Is the Control key pressed?
    bool shift;    ///< Is the Shift key pressed?
    bool system;   ///< Is the System key pressed?
};

/**
 * @brief Mouse button event data.
 *
 * Contains information about a mouse button event.
 */
struct MouseButtonEvent {
    MouseButton button;  ///< Code of the button that has been pressed
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
 * @brief Generic event structure.
 *
 * Encapsulates all types of events that can occur.
 * The type field indicates which union member is valid.
 */
struct Event {
    EventType type;  ///< Type of the event

    union {
        SizeEvent size;                    ///< Size event parameters
        KeyEvent key;                      ///< Key event parameters
        TextEvent text;                    ///< Text event parameters
        MouseMoveEvent mouseMove;          ///< Mouse move event parameters
        MouseButtonEvent mouseButton;      ///< Mouse button event parameters
        MouseWheelScrollEvent mouseWheel;  ///< Mouse wheel event parameters
    };
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_EVENT_HPP_
