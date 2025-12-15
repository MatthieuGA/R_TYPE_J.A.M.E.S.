/**
 * @file AudioTypes.hpp
 * @brief Common types and enums for the audio subsystem.
 */

#pragma once
#include <string>

namespace Rtype::Client::Audio {

/**
 * @brief Audio category for volume and mute control.
 */
enum class SoundCategory {
    SFX,   ///< Sound effects category
    MUSIC  ///< Background music category
};

/**
 * @brief Playback request data.
 */
struct PlaybackRequest {
    std::string id;
    float volume = 1.0f;
    bool loop = false;
    SoundCategory category = SoundCategory::SFX;
};

}  // namespace Rtype::Client::Audio
