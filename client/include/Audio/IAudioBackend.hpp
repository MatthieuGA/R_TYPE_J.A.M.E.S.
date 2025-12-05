/**
 * @file IAudioBackend.hpp
 * @brief Abstract interface for audio backend implementations.
 */

#pragma once
#include <string>

#include "AudioTypes.hpp"

namespace Rtype::Client::Audio {

/**
 * @brief Abstract interface for audio backend.
 *
 * This interface defines the contract for audio backends.
 * The only class allowed to use SFML Audio directly is the concrete backend.
 */
class IAudioBackend {
 public:
    virtual ~IAudioBackend() = default;

    /**
     * @brief Load a sound asset.
     *
     * @param id Unique identifier for the sound.
     * @param path File path to the sound asset.
     * @return True if loading succeeded, false otherwise.
     */
    virtual bool LoadSound(const std::string &id, const std::string &path) = 0;

    /**
     * @brief Load a music asset.
     *
     * @param id Unique identifier for the music.
     * @param path File path to the music asset.
     * @return True if loading succeeded, false otherwise.
     */
    virtual bool LoadMusic(const std::string &id, const std::string &path) = 0;

    /**
     * @brief Play a sound or music.
     *
     * @param request Playback request containing id, volume, loop, and
     * category.
     */
    virtual void Play(const PlaybackRequest &request) = 0;

    /**
     * @brief Stop currently playing music.
     */
    virtual void StopMusic() = 0;

    /**
     * @brief Set volume for a specific category.
     *
     * @param category The sound category (SFX or MUSIC).
     * @param volume Volume level (0.0 to 1.0).
     */
    virtual void SetCategoryVolume(SoundCategory category, float volume) = 0;

    /**
     * @brief Mute or unmute a specific category.
     *
     * @param category The sound category (SFX or MUSIC).
     * @param mute True to mute, false to unmute.
     */
    virtual void SetCategoryMute(SoundCategory category, bool mute) = 0;

    /**
     * @brief Update the backend (e.g., process queued commands).
     *
     * Should be called regularly from the main thread.
     */
    virtual void Update() = 0;
};

}  // namespace Rtype::Client::Audio
