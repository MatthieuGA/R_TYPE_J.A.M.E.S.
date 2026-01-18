/**
 * @file SettingsManager.hpp
 * @brief Persistent storage manager for all user settings.
 *
 * This class handles loading and saving ALL user settings to a single JSON
 * file. It does not change how settings are used - it only provides
 * persistence.
 *
 * Design decisions:
 * - Format: JSON (human-readable, debuggable, forward-compatible)
 * - Location: config/settings.json (relative to working directory)
 * - Loading: Once at startup, populates existing settings structures
 * - Saving: On demand (when settings change) or at shutdown
 * - Error handling: Missing file → use defaults, log warning, never crash
 * - Threading: Single-threaded only (no async I/O)
 *
 * Failure handling:
 * - Missing file: Create with defaults
 * - Invalid JSON: Log error, use defaults
 * - Missing keys: Use default values for those keys
 * - Unknown keys: Ignore (forward compatibility)
 * - Write failure: Log error, continue execution
 */

#pragma once

#include <cstdint>
#include <string>

#include "include/AccessibilitySettings.hpp"
#include "include/GameplaySettings.hpp"
#include "include/GraphicsSettings.hpp"
#include "include/input/InputManager.hpp"
#include "include/input/Key.hpp"

namespace Rtype::Client {

// Forward declarations
enum class ActionT;  // Will be Game::Action

/**
 * @brief Manages persistent storage of user settings.
 *
 * Usage:
 * 1. At startup: Load() to populate settings structures
 * 2. On change: Save() to persist current state
 * 3. At shutdown: Save() to ensure state is persisted
 *
 * This class does NOT own settings - it reads from and writes to
 * settings structures owned by GameWorld or other components.
 */
class SettingsManager {
 public:
    /**
     * @brief Construct a settings manager.
     *
     * @param file_path Path to settings file (relative or absolute)
     *                  Default: "config/settings.json"
     */
    explicit SettingsManager(
        const std::string &file_path = "config/settings.json");

    /**
     * @brief Load all settings from disk.
     *
     * Populates the provided settings structures with values from file.
     * If file is missing or invalid, settings are left at their current
     * (default) values and a warning is logged.
     *
     * @param gameplay Gameplay settings to populate
     * @param accessibility Accessibility settings to populate
     * @param graphics Graphics settings to populate
     * @param input_manager Input manager to populate with key bindings
     * @return true if loaded successfully, false if file missing/invalid
     */
    template <typename ActionT>
    bool Load(GameplaySettings &gameplay, AccessibilitySettings &accessibility,
        GraphicsSettings &graphics,
        Engine::Input::InputManager<ActionT> &input_manager);

    /**
     * @brief Save all settings to disk.
     *
     * Writes current state of all settings to the JSON file.
     * Creates parent directory if needed.
     * Uses atomic write (write to temp, then rename) to prevent corruption.
     *
     * @param gameplay Current gameplay settings
     * @param accessibility Current accessibility settings
     * @param graphics Current graphics settings
     * @param input_manager Current input manager (for key bindings)
     * @return true if saved successfully, false on error
     */
    template <typename ActionT>
    bool Save(const GameplaySettings &gameplay,
        const AccessibilitySettings &accessibility,
        const GraphicsSettings &graphics,
        const Engine::Input::InputManager<ActionT> &input_manager) const;

    /**
     * @brief Get the settings file path.
     * @return const std::string& The file path
     */
    const std::string &GetFilePath() const {
        return file_path_;
    }

 private:
    std::string file_path_;

    // Helper to ensure directory exists
    bool EnsureDirectoryExists(const std::string &path) const;
};

}  // namespace Rtype::Client

// Template implementation must be in header
#include "SettingsManager.tpp"
