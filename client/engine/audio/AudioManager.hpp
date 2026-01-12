/**
 * @file AudioManager.hpp
 * @brief High-level audio manager for ECS systems.
 */

#pragma once
#include <memory>
#include <string>

#include "include/audio/IAudioBackend.hpp"

namespace Rtype::Client::Audio {

/**
 * @brief High-level audio manager.
 *
 * This class wraps the audio backend and provides a simplified API
 * for ECS systems. It does NOT know about SFML.
 */
class AudioManager {
 public:
    /**
     * @brief Construct AudioManager with a backend.
     *
     * @param backend The audio backend to use (e.g., SFMLAudioBackend).
     */
    explicit AudioManager(std::unique_ptr<IAudioBackend> backend);
    ~AudioManager();

    /**
     * @brief Register an audio asset.
     *
     * @param id Unique identifier for the asset.
     * @param path File path to the asset.
     * @param is_music True if this is background music, false for SFX.
     * @return True if registration succeeded.
     */
    bool RegisterAsset(
        const std::string &id, const std::string &path, bool is_music);

    /**
     * @brief Play a sound effect.
     *
     * @param id Sound asset identifier.
     * @param volume Volume level (0.0 to 1.0, default 1.0).
     */
    void PlaySound(const std::string &id, float volume = 1.0f);

    /**
     * @brief Play background music.
     *
     * @param id Music asset identifier.
     * @param loop True to loop the music (default true).
     */
    void PlayMusic(const std::string &id, bool loop = true);

    /**
     * @brief Stop currently playing music.
     */
    void StopMusic();

    /**
     * @brief Check if a specific music track is currently playing.
     *
     * @param id Music asset identifier.
     * @return True if the music is playing, false otherwise.
     */
    bool IsMusicPlaying(const std::string &id) const;

    /**
     * @brief Set SFX volume.
     *
     * @param volume Volume level (0.0 to 1.0).
     */
    void SetSfxVolume(float volume);

    /**
     * @brief Set music volume.
     *
     * @param volume Volume level (0.0 to 1.0).
     */
    void SetMusicVolume(float volume);

    /**
     * @brief Mute or unmute SFX.
     *
     * @param mute True to mute, false to unmute.
     */
    void MuteSfx(bool mute);

    /**
     * @brief Mute or unmute music.
     *
     * @param mute True to mute, false to unmute.
     */
    void MuteMusic(bool mute);

    /**
     * @brief Update the audio system.
     *
     * Should be called regularly to process queued audio commands.
     */
    void Update();

    /**
     * @brief Get current SFX volume.
     *
     * @return Current SFX volume level (0.0 to 1.0).
     */
    float GetSfxVolume() const;

    /**
     * @brief Check if SFX is muted.
     *
     * @return True if SFX is muted, false otherwise.
     */
    bool IsSfxMuted() const;

    /**
     * @brief Get current music volume.
     *
     * @return Current music volume level (0.0 to 1.0).
     */
    float GetMusicVolume() const;

    /**
     * @brief Check if music is muted.
     *
     * @return True if music is muted, false otherwise.
     */
    bool IsMusicMuted() const;

 private:
    std::unique_ptr<IAudioBackend> backend_;
};

}  // namespace Rtype::Client::Audio
