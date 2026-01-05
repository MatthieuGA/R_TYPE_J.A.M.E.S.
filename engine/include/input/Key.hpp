/**
 * @file Key.hpp
 * @brief Backend-agnostic keyboard key enumeration for the Engine.
 *
 * Provides an engine-agnostic representation of keyboard keys.
 * This allows the client to use input abstractions independent of the
 * underlying input library (e.g., SFML).
 */

#ifndef ENGINE_INCLUDE_INPUT_KEY_HPP_
#define ENGINE_INCLUDE_INPUT_KEY_HPP_

namespace Engine {
namespace Input {

/**
 * @brief Enumeration of keyboard keys.
 *
 * Represents commonly used keyboard keys in a backend-agnostic manner.
 * Values are chosen to match common usage patterns in the codebase.
 */
enum class Key {
    Unknown = -1,  ///< Unhandled key

    // Letters
    A = 0,  ///< The A key
    B,      ///< The B key
    C,      ///< The C key
    D,      ///< The D key
    E,      ///< The E key
    F,      ///< The F key
    G,      ///< The G key
    H,      ///< The H key
    I,      ///< The I key
    J,      ///< The J key
    K,      ///< The K key
    L,      ///< The L key
    M,      ///< The M key
    N,      ///< The N key
    O,      ///< The O key
    P,      ///< The P key
    Q,      ///< The Q key
    R,      ///< The R key
    S,      ///< The S key
    T,      ///< The T key
    U,      ///< The U key
    V,      ///< The V key
    W,      ///< The W key
    X,      ///< The X key
    Y,      ///< The Y key
    Z,      ///< The Z key

    // Numbers
    Num0,  ///< The 0 key
    Num1,  ///< The 1 key
    Num2,  ///< The 2 key
    Num3,  ///< The 3 key
    Num4,  ///< The 4 key
    Num5,  ///< The 5 key
    Num6,  ///< The 6 key
    Num7,  ///< The 7 key
    Num8,  ///< The 8 key
    Num9,  ///< The 9 key

    // Arrow keys
    Left,   ///< The left arrow key
    Right,  ///< The right arrow key
    Up,     ///< The up arrow key
    Down,   ///< The down arrow key

    // Special keys
    Space,      ///< The space bar
    Enter,      ///< The Enter key
    Escape,     ///< The Escape key
    Tab,        ///< The Tab key
    Backspace,  ///< The Backspace key
    Delete,     ///< The Delete key

    // Function keys
    F1,   ///< The F1 key
    F2,   ///< The F2 key
    F3,   ///< The F3 key
    F4,   ///< The F4 key
    F5,   ///< The F5 key
    F6,   ///< The F6 key
    F7,   ///< The F7 key
    F8,   ///< The F8 key
    F9,   ///< The F9 key
    F10,  ///< The F10 key
    F11,  ///< The F11 key
    F12,  ///< The F12 key

    // Modifiers
    LShift,    ///< The left Shift key
    RShift,    ///< The right Shift key
    LControl,  ///< The left Control key
    RControl,  ///< The right Control key
    LAlt,      ///< The left Alt key
    RAlt,      ///< The right Alt key

    KeyCount  ///< Keep last -- the total number of keys
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_KEY_HPP_
