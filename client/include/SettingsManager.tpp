/**
 * @file SettingsManager.tpp
 * @brief Template implementation for SettingsManager.
 *
 * This file contains the template methods for loading/saving settings.
 * It is included by SettingsManager.hpp and should not be included directly.
 */

#pragma once

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "game/GameAction.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Rtype::Client {

using json = nlohmann::json;

// =============================================================================
// JSON Serialization Helpers
// =============================================================================

/**
 * @brief Convert DifficultyLevel to string.
 */
inline std::string DifficultyToString(DifficultyLevel level) {
    switch (level) {
        case DifficultyLevel::Easy:
            return "Easy";
        case DifficultyLevel::Normal:
            return "Normal";
        case DifficultyLevel::Hard:
            return "Hard";
        default:
            return "Normal";
    }
}

/**
 * @brief Convert string to DifficultyLevel.
 */
inline DifficultyLevel StringToDifficulty(const std::string &str) {
    if (str == "Easy")
        return DifficultyLevel::Easy;
    if (str == "Hard")
        return DifficultyLevel::Hard;
    return DifficultyLevel::Normal;
}

/**
 * @brief Convert TextSizeScale to string.
 */
inline std::string TextSizeToString(TextSizeScale scale) {
    switch (scale) {
        case TextSizeScale::Small:
            return "Small";
        case TextSizeScale::Large:
            return "Large";
        default:
            return "Normal";
    }
}

/**
 * @brief Convert string to TextSizeScale.
 */
inline TextSizeScale StringToTextSize(const std::string &str) {
    if (str == "Small")
        return TextSizeScale::Small;
    if (str == "Large")
        return TextSizeScale::Large;
    return TextSizeScale::Normal;
}

/**
 * @brief Convert WindowMode to string.
 */
inline std::string WindowModeToString(WindowMode mode) {
    switch (mode) {
        case WindowMode::Fullscreen:
            return "Fullscreen";
        case WindowMode::Borderless:
            return "Borderless";
        default:
            return "Windowed";
    }
}

/**
 * @brief Convert string to WindowMode.
 */
inline WindowMode StringToWindowMode(const std::string &str) {
    if (str == "Fullscreen")
        return WindowMode::Fullscreen;
    if (str == "Borderless")
        return WindowMode::Borderless;
    return WindowMode::Windowed;
}

/**
 * @brief Convert AntiAliasingLevel to int.
 */
inline int AALevelToInt(AntiAliasingLevel level) {
    return static_cast<int>(level);
}

/**
 * @brief Convert int to AntiAliasingLevel.
 */
inline AntiAliasingLevel IntToAALevel(int value) {
    switch (value) {
        case 2:
            return AntiAliasingLevel::AA2x;
        case 4:
            return AntiAliasingLevel::AA4x;
        case 8:
            return AntiAliasingLevel::AA8x;
        default:
            return AntiAliasingLevel::Off;
    }
}

/**
 * @brief Convert Key enum to string name.
 */
inline std::string KeyToString(Engine::Input::Key key) {
    // Simple implementation - stores numeric value
    // Could be enhanced to store human-readable names
    return std::to_string(static_cast<int>(key));
}

/**
 * @brief Convert string to Key enum.
 */
inline Engine::Input::Key StringToKey(const std::string &str) {
    try {
        int value = std::stoi(str);
        return static_cast<Engine::Input::Key>(value);
    } catch (...) {
        return Engine::Input::Key::Unknown;
    }
}

/**
 * @brief Convert MouseButton enum to string.
 */
inline std::string MouseButtonToString(Engine::Input::MouseButton button) {
    return std::to_string(static_cast<int>(button));
}

/**
 * @brief Convert string to MouseButton enum.
 */
inline Engine::Input::MouseButton StringToMouseButton(const std::string &str) {
    try {
        int value = std::stoi(str);
        return static_cast<Engine::Input::MouseButton>(value);
    } catch (...) {
        return Engine::Input::MouseButton::Left;
    }
}

/**
 * @brief Convert Game::Action to string name.
 */
inline std::string ActionToString(Game::Action action) {
    switch (action) {
        case Game::Action::MoveUp:
            return "MoveUp";
        case Game::Action::MoveDown:
            return "MoveDown";
        case Game::Action::MoveLeft:
            return "MoveLeft";
        case Game::Action::MoveRight:
            return "MoveRight";
        case Game::Action::Shoot:
            return "Shoot";
        case Game::Action::ChargeShoot:
            return "ChargeShoot";
        case Game::Action::Confirm:
            return "Confirm";
        case Game::Action::Cancel:
            return "Cancel";
        case Game::Action::Pause:
            return "Pause";
        case Game::Action::MenuUp:
            return "MenuUp";
        case Game::Action::MenuDown:
            return "MenuDown";
        case Game::Action::MenuLeft:
            return "MenuLeft";
        case Game::Action::MenuRight:
            return "MenuRight";
        default:
            return "Unknown";
    }
}

/**
 * @brief Convert string to Game::Action.
 */
inline Game::Action StringToAction(const std::string &str) {
    if (str == "MoveUp")
        return Game::Action::MoveUp;
    if (str == "MoveDown")
        return Game::Action::MoveDown;
    if (str == "MoveLeft")
        return Game::Action::MoveLeft;
    if (str == "MoveRight")
        return Game::Action::MoveRight;
    if (str == "Shoot")
        return Game::Action::Shoot;
    if (str == "ChargeShoot")
        return Game::Action::ChargeShoot;
    if (str == "Confirm")
        return Game::Action::Confirm;
    if (str == "Cancel")
        return Game::Action::Cancel;
    if (str == "Pause")
        return Game::Action::Pause;
    if (str == "MenuUp")
        return Game::Action::MenuUp;
    if (str == "MenuDown")
        return Game::Action::MenuDown;
    if (str == "MenuLeft")
        return Game::Action::MenuLeft;
    if (str == "MenuRight")
        return Game::Action::MenuRight;
    return Game::Action::Count;  // Invalid
}

// =============================================================================
// Template Method Implementations
// =============================================================================

template <typename ActionT>
bool SettingsManager::Load(GameplaySettings &gameplay,
    AccessibilitySettings &accessibility, GraphicsSettings &graphics,
    Engine::Input::InputManager<ActionT> &input_manager) {
    std::ifstream file(file_path_);
    if (!file.is_open()) {
        std::cout << "[SettingsManager] No settings file found at "
                  << file_path_ << ", using defaults" << std::endl;
        return false;
    }

    try {
        json j;
        file >> j;

        // Load Gameplay Settings
        if (j.contains("gameplay")) {
            const auto &g = j["gameplay"];
            if (g.contains("game_speed"))
                gameplay.game_speed = g["game_speed"].get<float>();
            if (g.contains("auto_fire_enabled"))
                gameplay.auto_fire_enabled = g["auto_fire_enabled"].get<bool>();
            if (g.contains("killable_enemy_projectiles"))
                gameplay.killable_enemy_projectiles =
                    g["killable_enemy_projectiles"].get<bool>();
            if (g.contains("difficulty"))
                gameplay.difficulty =
                    StringToDifficulty(g["difficulty"].get<std::string>());
        }

        // Load Accessibility Settings
        if (j.contains("accessibility")) {
            const auto &a = j["accessibility"];
            if (a.contains("high_contrast"))
                accessibility.high_contrast = a["high_contrast"].get<bool>();
            if (a.contains("text_scale"))
                accessibility.text_scale =
                    StringToTextSize(a["text_scale"].get<std::string>());
            if (a.contains("reduced_visuals"))
                accessibility.reduced_visuals =
                    a["reduced_visuals"].get<bool>();
        }

        // Load Graphics Settings
        if (j.contains("graphics")) {
            const auto &gr = j["graphics"];
            if (gr.contains("resolution_width"))
                graphics.resolution_width =
                    gr["resolution_width"].get<uint16_t>();
            if (gr.contains("resolution_height"))
                graphics.resolution_height =
                    gr["resolution_height"].get<uint16_t>();
            if (gr.contains("window_mode"))
                graphics.window_mode =
                    StringToWindowMode(gr["window_mode"].get<std::string>());
            if (gr.contains("vsync_enabled"))
                graphics.vsync_enabled = gr["vsync_enabled"].get<bool>();
            if (gr.contains("frame_rate_limit"))
                graphics.frame_rate_limit =
                    gr["frame_rate_limit"].get<uint16_t>();
            if (gr.contains("anti_aliasing"))
                graphics.anti_aliasing =
                    IntToAALevel(gr["anti_aliasing"].get<int>());

            // Copy to pending settings
            graphics.pending_resolution_width = graphics.resolution_width;
            graphics.pending_resolution_height = graphics.resolution_height;
            graphics.pending_window_mode = graphics.window_mode;
            graphics.pending_anti_aliasing = graphics.anti_aliasing;
        }

        // Load Input Bindings
        if (j.contains("input_bindings")) {
            const auto &bindings = j["input_bindings"];
            
            // Clear existing bindings before loading
            input_manager.ClearAllBindings();

            for (const auto &[action_name, binding_array] : bindings.items()) {
                ActionT action = static_cast<ActionT>(
                    StringToAction(action_name));
                
                if (action == static_cast<ActionT>(Game::Action::Count)) {
                    continue;  // Skip invalid actions
                }

                for (const auto &binding_obj : binding_array) {
                    if (!binding_obj.contains("type"))
                        continue;

                    std::string type = binding_obj["type"].get<std::string>();
                    
                    if (type == "key" && binding_obj.contains("value")) {
                        Engine::Input::Key key =
                            StringToKey(binding_obj["value"].get<std::string>());
                        input_manager.BindKey(action, key);
                    } else if (type == "mouse" &&
                               binding_obj.contains("value")) {
                        Engine::Input::MouseButton button =
                            StringToMouseButton(
                                binding_obj["value"].get<std::string>());
                        input_manager.BindMouseButton(action, button);
                    }
                }
            }
        }

        std::cout << "[SettingsManager] Loaded settings from " << file_path_
                  << std::endl;
        
        // Display loaded settings for debugging
        std::cout << "  [Gameplay] Speed: " << gameplay.game_speed
                  << ", Auto-fire: " << (gameplay.auto_fire_enabled ? "ON" : "OFF")
                  << ", Killable projectiles: " << (gameplay.killable_enemy_projectiles ? "ON" : "OFF")
                  << ", Difficulty: " << DifficultyToString(gameplay.difficulty)
                  << std::endl;
        
        std::cout << "  [Accessibility] High contrast: " << (accessibility.high_contrast ? "ON" : "OFF")
                  << ", Text scale: " << TextSizeToString(accessibility.text_scale)
                  << ", Reduced visuals: " << (accessibility.reduced_visuals ? "ON" : "OFF")
                  << std::endl;
        
        std::cout << "  [Graphics] Resolution: " << graphics.resolution_width << "x" << graphics.resolution_height
                  << ", Window mode: " << WindowModeToString(graphics.window_mode)
                  << ", VSync: " << (graphics.vsync_enabled ? "ON" : "OFF")
                  << ", FPS limit: " << graphics.frame_rate_limit
                  << ", Anti-aliasing: " << AALevelToInt(graphics.anti_aliasing) << "x"
                  << std::endl;
        
        std::cout << "  [Input Bindings] Loaded " 
                  << (j.contains("input_bindings") ? j["input_bindings"].size() : 0) 
                  << " action(s)" << std::endl;
        
        return true;

    } catch (const std::exception &e) {
        std::cerr << "[SettingsManager] Error loading settings: " << e.what()
                  << std::endl;
        std::cerr << "[SettingsManager] Using default settings" << std::endl;
        return false;
    }
}

template <typename ActionT>
bool SettingsManager::Save(const GameplaySettings &gameplay,
    const AccessibilitySettings &accessibility,
    const GraphicsSettings &graphics,
    const Engine::Input::InputManager<ActionT> &input_manager) const {
    // Ensure directory exists
    size_t last_slash = file_path_.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        std::string dir = file_path_.substr(0, last_slash);
        if (!EnsureDirectoryExists(dir)) {
            std::cerr << "[SettingsManager] Failed to create directory: "
                      << dir << std::endl;
            return false;
        }
    }

    try {
        json j;

        // Save Gameplay Settings
        j["gameplay"]["game_speed"] = gameplay.game_speed;
        j["gameplay"]["auto_fire_enabled"] = gameplay.auto_fire_enabled;
        j["gameplay"]["killable_enemy_projectiles"] =
            gameplay.killable_enemy_projectiles;
        j["gameplay"]["difficulty"] = DifficultyToString(gameplay.difficulty);

        // Save Accessibility Settings
        j["accessibility"]["high_contrast"] = accessibility.high_contrast;
        j["accessibility"]["text_scale"] =
            TextSizeToString(accessibility.text_scale);
        j["accessibility"]["reduced_visuals"] = accessibility.reduced_visuals;

        // Save Graphics Settings
        j["graphics"]["resolution_width"] = graphics.resolution_width;
        j["graphics"]["resolution_height"] = graphics.resolution_height;
        j["graphics"]["window_mode"] = WindowModeToString(graphics.window_mode);
        j["graphics"]["vsync_enabled"] = graphics.vsync_enabled;
        j["graphics"]["frame_rate_limit"] = graphics.frame_rate_limit;
        j["graphics"]["anti_aliasing"] = AALevelToInt(graphics.anti_aliasing);

        // Save Input Bindings
        json bindings_json;
        
        // Iterate through all possible actions
        for (size_t i = 0; i < static_cast<size_t>(ActionT::Count); ++i) {
            ActionT action = static_cast<ActionT>(i);
            const auto &bindings = input_manager.GetBindings(action);
            
            if (bindings.empty()) {
                continue;  // Don't save empty bindings
            }

            std::string action_name = ActionToString(
                static_cast<Game::Action>(action));
            json binding_array = json::array();

            for (const auto &binding : bindings) {
                json binding_obj;
                if (binding.type ==
                    Engine::Input::InputBinding::Type::Key) {
                    binding_obj["type"] = "key";
                    binding_obj["value"] = KeyToString(binding.key);
                } else if (binding.type ==
                           Engine::Input::InputBinding::Type::MouseButton) {
                    binding_obj["type"] = "mouse";
                    binding_obj["value"] =
                        MouseButtonToString(binding.mouse_button);
                }
                binding_array.push_back(binding_obj);
            }

            bindings_json[action_name] = binding_array;
        }
        
        j["input_bindings"] = bindings_json;

        // Write to temp file first (atomic write)
        std::string temp_path = file_path_ + ".tmp";
        std::ofstream temp_file(temp_path);
        if (!temp_file.is_open()) {
            std::cerr << "[SettingsManager] Failed to open temp file: "
                      << temp_path << std::endl;
            return false;
        }

        temp_file << j.dump(2);  // Pretty-print with 2-space indent
        temp_file.close();

        // Rename temp to actual file (atomic on POSIX)
        if (std::rename(temp_path.c_str(), file_path_.c_str()) != 0) {
            std::cerr << "[SettingsManager] Failed to rename temp file"
                      << std::endl;
            return false;
        }

        std::cout << "[SettingsManager] Saved settings to " << file_path_
                  << std::endl;
        return true;

    } catch (const std::exception &e) {
        std::cerr << "[SettingsManager] Error saving settings: " << e.what()
                  << std::endl;
        return false;
    }
}

}  // namespace Rtype::Client
