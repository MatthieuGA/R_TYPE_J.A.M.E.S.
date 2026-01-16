/**
 * @file IWindow.hpp
 * @brief Abstract window interface for backend-agnostic window management.
 *
 * This interface defines the contract for window implementations.
 * It wraps OS window lifecycle operations without exposing backend details.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_IWINDOW_HPP_
#define ENGINE_INCLUDE_GRAPHICS_IWINDOW_HPP_

#include <string>

#include "graphics/Types.hpp"

namespace Engine::Graphics {

/**
 * @brief Abstract window interface.
 *
 * Provides minimal window lifecycle management without backend dependencies.
 * Backend implementations (SFML, SDL, etc.) handle the actual window creation.
 */
class IWindow {
 public:
    virtual ~IWindow() = default;

    /**
     * @brief Check if the window is currently open.
     * @return true if the window is open, false otherwise
     */
    virtual bool IsOpen() const = 0;

    /**
     * @brief Close the window.
     *
     * After calling this, IsOpen() should return false.
     */
    virtual void Close() = 0;

    /**
     * @brief Get the current window size.
     * @return Window dimensions in pixels
     */
    virtual Vector2i GetSize() const = 0;

    /**
     * @brief Set the window title.
     * @param title New window title
     */
    virtual void SetTitle(const std::string &title) = 0;

    /**
     * @brief Display the window contents (swap buffers).
     *
     * Presents the rendered frame to the screen.
     */
    virtual void Display() = 0;

    /**
     * @brief Check if the window currently has input focus.
     * @return true if the window has focus, false otherwise
     */
    virtual bool HasFocus() const = 0;

    /**
     * @brief Convert pixel coordinates to world/view coordinates.
     *
     * This is useful for mapping mouse positions from screen space
     * to game world space, accounting for camera transforms.
     *
     * @param pixel Pixel coordinates (screen space)
     * @return World coordinates (view space)
     */
    virtual Vector2f MapPixelToCoords(const Vector2i &pixel) const = 0;
};

}  // namespace Engine::Graphics

#endif  // ENGINE_INCLUDE_GRAPHICS_IWINDOW_HPP_
