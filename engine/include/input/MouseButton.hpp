/**
 * @file MouseButton.hpp
 * @brief Backend-agnostic mouse button enumeration for the Engine.
 *
 * Provides an engine-agnostic representation of mouse buttons.
 * This allows the client to use input abstractions independent of the
 * underlying input library (e.g., SFML).
 */

#ifndef ENGINE_INCLUDE_INPUT_MOUSEBUTTON_HPP_
#define ENGINE_INCLUDE_INPUT_MOUSEBUTTON_HPP_

namespace Engine {
namespace Input {

/**
 * @brief Enumeration of mouse buttons.
 *
 * Represents commonly used mouse buttons in a backend-agnostic manner.
 */
enum class MouseButton {
    Left = 0,  ///< The left mouse button
    Right,     ///< The right mouse button
    Middle,    ///< The middle (wheel) mouse button
    XButton1,  ///< The first extra mouse button
    XButton2,  ///< The second extra mouse button

    ButtonCount  ///< Keep last -- the total number of mouse buttons
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_MOUSEBUTTON_HPP_
