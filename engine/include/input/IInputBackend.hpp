/**
 * @file IInputBackend.hpp
 * @brief Abstract interface for input backend implementations.
 *
 * This interface defines the contract that input backends must implement
 * to provide raw input state. Backends are responsible for polling the
 * underlying input system (SFML, SDL, etc.) and exposing the state
 * through this interface.
 *
 * The InputManager uses this interface to query physical input state
 * and translate it to logical actions.
 */

#ifndef ENGINE_INCLUDE_INPUT_IINPUTBACKEND_HPP_
#define ENGINE_INCLUDE_INPUT_IINPUTBACKEND_HPP_

#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Engine {
namespace Input {

/**
 * @brief Mouse position in screen coordinates.
 */
struct MousePosition {
    int x;  ///< X position relative to window left
    int y;  ///< Y position relative to window top
};

/**
 * @brief Abstract interface for input backend implementations.
 *
 * Provides raw input state queries for keyboard, mouse, and other
 * input devices. Implementations are backend-specific (SFML, SDL, etc.)
 * but this interface is backend-agnostic.
 *
 * Thread Safety: Implementations are NOT required to be thread-safe.
 * All input queries should occur on the main thread.
 */
class IInputBackend {
 public:
    virtual ~IInputBackend() = default;

    // =========================================================================
    // Keyboard State
    // =========================================================================

    /**
     * @brief Check if a keyboard key is currently pressed.
     *
     * @param key The key to check
     * @return true if the key is currently held down
     */
    virtual bool IsKeyPressed(Key key) const = 0;

    // =========================================================================
    // Mouse State
    // =========================================================================

    /**
     * @brief Check if a mouse button is currently pressed.
     *
     * @param button The mouse button to check
     * @return true if the button is currently held down
     */
    virtual bool IsMouseButtonPressed(MouseButton button) const = 0;

    /**
     * @brief Get the current mouse position in screen coordinates.
     *
     * @return MousePosition The current mouse position
     */
    virtual MousePosition GetMousePosition() const = 0;

    /**
     * @brief Get the mouse position relative to a specific window.
     *
     * This is useful when dealing with multiple windows or when
     * the mouse position needs to be in window-local coordinates.
     *
     * @return MousePosition The mouse position relative to the active window
     */
    virtual MousePosition GetMousePositionInWindow() const = 0;

    // =========================================================================
    // Focus State
    // =========================================================================

    /**
     * @brief Check if the input window currently has focus.
     *
     * Input should typically be ignored when the window lacks focus.
     *
     * @return true if the window has keyboard focus
     */
    virtual bool HasWindowFocus() const = 0;
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_IINPUTBACKEND_HPP_
