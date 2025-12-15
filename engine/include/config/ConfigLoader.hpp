/**
 * @file ConfigLoader.hpp
 * @brief Simple JSON config parser for engine configuration.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Engine {

/**
 * @brief Simple config loader for plugin paths.
 *
 * Parses a minimal JSON config to extract plugin backend paths.
 * This is a lightweight parser - not a full JSON implementation.
 */
class ConfigLoader {
 public:
    /**
     * @brief Load configuration from file.
     *
     * @param config_path Path to engine_config.json
     * @return true if loaded successfully
     */
    static bool Load(const std::string &config_path) {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "[ConfigLoader] Failed to open: " << config_path
                      << std::endl;
            return false;
        }

        std::string line;
        std::string current_section;

        while (std::getline(file, line)) {
            // Remove whitespace
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);

            // Track sections
            if (line.find("\"video\"") != std::string::npos) {
                current_section = "video";
            } else if (line.find("\"audio\"") != std::string::npos) {
                current_section = "audio";
            }

            // Extract backend paths
            if (line.find("\"backend\"") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string value = line.substr(colon + 1);
                    // Remove quotes, commas, whitespace
                    value.erase(0, value.find_first_not_of(" \t\""));
                    value.erase(value.find_last_not_of(" \t\",") + 1);

                    if (current_section == "video") {
                        video_backend_ = value;
                    } else if (current_section == "audio") {
                        audio_backend_ = value;
                    }
                }
            }

            // Extract audio settings
            if (current_section == "audio") {
                if (line.find("\"sfx_volume\"") != std::string::npos) {
                    sfx_volume_ = ExtractFloat(line);
                } else if (line.find("\"music_volume\"") !=
                           std::string::npos) {
                    music_volume_ = ExtractFloat(line);
                } else if (line.find("\"mute_sfx\"") != std::string::npos) {
                    mute_sfx_ = ExtractBool(line);
                } else if (line.find("\"mute_music\"") != std::string::npos) {
                    mute_music_ = ExtractBool(line);
                }
            }

            // Extract window settings
            if (current_section == "video") {
                if (line.find("\"width\"") != std::string::npos) {
                    window_width_ = ExtractInt(line);
                } else if (line.find("\"height\"") != std::string::npos) {
                    window_height_ = ExtractInt(line);
                } else if (line.find("\"title\"") != std::string::npos) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string value = line.substr(colon + 1);
                        value.erase(0, value.find_first_not_of(" \t\""));
                        value.erase(value.find_last_not_of(" \t\",") + 1);
                        window_title_ = value;
                    }
                }
            }
        }

        std::cout << "[ConfigLoader] Loaded configuration:" << std::endl;
        std::cout << "  Video backend: " << video_backend_ << std::endl;
        std::cout << "  Audio backend: " << audio_backend_ << std::endl;
        std::cout << "  Window: " << window_width_ << "x" << window_height_
                  << " - " << window_title_ << std::endl;

        return !video_backend_.empty() && !audio_backend_.empty();
    }

    static const std::string &GetVideoBackend() {
        return video_backend_;
    }

    static const std::string &GetAudioBackend() {
        return audio_backend_;
    }

    static float GetSfxVolume() {
        return sfx_volume_;
    }

    static float GetMusicVolume() {
        return music_volume_;
    }

    static bool GetMuteSfx() {
        return mute_sfx_;
    }

    static bool GetMuteMusic() {
        return mute_music_;
    }

    static unsigned int GetWindowWidth() {
        return window_width_;
    }

    static unsigned int GetWindowHeight() {
        return window_height_;
    }

    static const std::string &GetWindowTitle() {
        return window_title_;
    }

 private:
    static float ExtractFloat(const std::string &line) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string value = line.substr(colon + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t,") + 1);
            return std::stof(value);
        }
        return 0.0f;
    }

    static int ExtractInt(const std::string &line) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string value = line.substr(colon + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t,") + 1);
            return std::stoi(value);
        }
        return 0;
    }

    static bool ExtractBool(const std::string &line) {
        return line.find("true") != std::string::npos;
    }

    inline static std::string video_backend_ = "../lib/sfml_video_module.so";
    inline static std::string audio_backend_ = "../lib/sfml_audio_module.so";
    inline static float sfx_volume_ = 0.7f;
    inline static float music_volume_ = 0.5f;
    inline static bool mute_sfx_ = false;
    inline static bool mute_music_ = false;
    inline static unsigned int window_width_ = 1920;
    inline static unsigned int window_height_ = 1080;
    inline static std::string window_title_ = "R-Type J.A.M.E.S.";
};

}  // namespace Engine
