/**
 * @file AccessibilitySettings.hpp
 * @brief Accessibility settings for the game (UI contrast, text size, visual
 * noise).
 */

#pragma once

namespace Rtype::Client {

/**
 * @brief Enum for text size scaling levels.
 */
enum class TextSizeScale {
    Small = 0,   ///< 0.8x normal size
    Normal = 1,  ///< 1.0x normal size (default)
    Large = 2    ///< 1.2x normal size
};

/**
 * @brief Container for accessibility settings.
 *
 * These settings apply to the UI only (menus, HUD) and do not affect gameplay.
 * All settings default to preserve current game behavior.
 */
struct AccessibilitySettings {
    /// Enable high-contrast mode for better readability
    bool high_contrast = false;

    /// Text size scaling for UI elements
    TextSizeScale text_scale = TextSizeScale::Normal;

    /// Reduce visual noise (disable background animations, etc.)
    bool reduced_visuals = false;

    /**
     * @brief Get the text scale multiplier based on current setting.
     * @return Scale multiplier (0.8, 1.0, or 1.2)
     */
    float GetTextScaleMultiplier() const {
        switch (text_scale) {
            case TextSizeScale::Small:
                return 0.8f;
            case TextSizeScale::Large:
                return 1.2f;
            default:
                return 1.0f;
        }
    }
};

}  // namespace Rtype::Client
