/**
 * @file GraphicsSettings.hpp
 * @brief Graphics settings for the game (resolution, window mode, VSync, etc).
 */

#pragma once

#include <cstdint>
#include <string>

namespace Rtype::Client {

/**
 * @brief Enum for window mode options.
 */
enum class WindowMode {
    Windowed = 0,    ///< Windowed mode
    Fullscreen = 1,  ///< Fullscreen mode
    Borderless = 2   ///< Borderless (fake fullscreen)
};

/**
 * @brief Enum for anti-aliasing levels.
 */
enum class AntiAliasingLevel {
    Off = 0,   ///< No anti-aliasing
    AA2x = 2,  ///< 2x MSAA
    AA4x = 4,  ///< 4x MSAA
    AA8x = 8   ///< 8x MSAA
};

/**
 * @brief Container for graphics settings.
 *
 * Some settings apply immediately (VSync, FPS limit).
 * Others require window recreation (resolution, window mode, anti-aliasing).
 * Store pending values and apply only on confirmation.
 */
struct GraphicsSettings {
    // Resolution (width, height)
    uint16_t resolution_width = 1920;
    uint16_t resolution_height = 1080;

    // Window mode
    WindowMode window_mode = WindowMode::Windowed;

    // VSync setting
    bool vsync_enabled = true;

    // Frame rate limit (0 = unlimited)
    uint16_t frame_rate_limit = 60;

    // Anti-aliasing level (requires window recreation)
    AntiAliasingLevel anti_aliasing = AntiAliasingLevel::Off;

    // Pending settings (stored but not yet applied)
    // Used for settings that require window recreation
    uint16_t pending_resolution_width = resolution_width;
    uint16_t pending_resolution_height = resolution_height;
    WindowMode pending_window_mode = window_mode;
    AntiAliasingLevel pending_anti_aliasing = anti_aliasing;

    /**
     * @brief Check if any pending setting differs from current setting.
     * @return True if changes are pending
     */
    bool HasPendingChanges() const {
        return (pending_resolution_width != resolution_width ||
                pending_resolution_height != resolution_height ||
                pending_window_mode != window_mode ||
                pending_anti_aliasing != anti_aliasing);
    }

    /**
     * @brief Apply all pending settings (window recreation required).
     */
    void ApplyPendingSettings() {
        resolution_width = pending_resolution_width;
        resolution_height = pending_resolution_height;
        window_mode = pending_window_mode;
        anti_aliasing = pending_anti_aliasing;
    }

    /**
     * @brief Discard pending changes and revert to current settings.
     */
    void DiscardPendingChanges() {
        pending_resolution_width = resolution_width;
        pending_resolution_height = resolution_height;
        pending_window_mode = window_mode;
        pending_anti_aliasing = anti_aliasing;
    }
};

}  // namespace Rtype::Client
