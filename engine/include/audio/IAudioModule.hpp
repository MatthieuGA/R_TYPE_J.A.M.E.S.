/**
 * @file IAudioModule.hpp
 * @brief Pure virtual interface for audio backend modules.
 *
 * This interface defines the contract that all audio backend plugins must
 * implement. Modules are loaded dynamically at runtime via shared libraries
 * (.so/.dll).
 */

#pragma once
#include <memory>
#include <string>

namespace Engine {
namespace Audio {

/**
 * @brief Playback request structure for audio operations.
 */
struct PlaybackRequest {
    std::string id;
    float volume = 1.0f;
    bool loop = false;

    enum class Category {
        SFX,
        MUSIC
    } category = Category::SFX;
};

/**
 * @brief Pure virtual interface for audio backend modules.
 *
 * All audio backend plugins (.so files) must implement this interface.
 * The engine loads these modules dynamically and communicates through this
 * interface.
 */
class IAudioModule {
 public:
    virtual ~IAudioModule() = default;

    /**
     * @brief Initialize the audio module.
     *
     * Called once when the module is loaded by the engine.
     *
     * @return True if initialization succeeded, false otherwise.
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Shutdown the audio module and release resources.
     *
     * Called when the engine is shutting down or switching backends.
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Update the audio module (e.g., stream updates, fade effects).
     *
     * Called every frame by the engine.
     *
     * @param delta_time Time elapsed since last update in seconds.
     */
    virtual void Update(float delta_time) = 0;

    /**
     * @brief Load a sound asset from file.
     *
     * @param id Unique identifier for this sound.
     * @param path File path to the sound asset.
     * @return True if loading succeeded, false otherwise.
     */
    virtual bool LoadSound(const std::string &id, const std::string &path) = 0;

    /**
     * @brief Load a music asset from file.
     *
     * @param id Unique identifier for this music.
     * @param path File path to the music asset.
     * @return True if loading succeeded, false otherwise.
     */
    virtual bool LoadMusic(const std::string &id, const std::string &path) = 0;

    /**
     * @brief Play a sound or music with the given parameters.
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
     * @brief Set the global volume for sound effects.
     *
     * @param volume Volume level (0.0 to 1.0).
     */
    virtual void SetSfxVolume(float volume) = 0;

    /**
     * @brief Set the global volume for music.
     *
     * @param volume Volume level (0.0 to 1.0).
     */
    virtual void SetMusicVolume(float volume) = 0;

    /**
     * @brief Mute or unmute sound effects.
     *
     * @param mute True to mute, false to unmute.
     */
    virtual void MuteSfx(bool mute) = 0;

    /**
     * @brief Mute or unmute music.
     *
     * @param mute True to mute, false to unmute.
     */
    virtual void MuteMusic(bool mute) = 0;

    /**
     * @brief Get the name/identifier of this audio module.
     *
     * @return Module name (e.g., "SFML Audio Backend", "OpenAL Backend").
     */
    virtual std::string GetModuleName() const = 0;
};

}  // namespace Audio
}  // namespace Engine
