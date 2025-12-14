/**
 * @file PluginAudioBackend.hpp
 * @brief Adapter to use IAudioModule plugins as IAudioBackend.
 */

#pragma once
#include <memory>
#include <string>

#include <audio/IAudioModule.hpp>

#include "include/audio/IAudioBackend.hpp"

namespace Rtype::Client::Audio {

/**
 * @brief Adapter that wraps an IAudioModule plugin to implement IAudioBackend.
 */
class PluginAudioBackend : public IAudioBackend {
 public:
    /**
     * @brief Construct the adapter with a plugin module.
     *
     * @param module The audio plugin module to wrap.
     */
    explicit PluginAudioBackend(
        std::shared_ptr<Engine::Audio::IAudioModule> module);
    ~PluginAudioBackend() override;

    bool LoadSound(const std::string &id, const std::string &path) override;
    bool LoadMusic(const std::string &id, const std::string &path) override;
    void Play(const PlaybackRequest &request) override;
    void StopMusic() override;
    void SetCategoryVolume(SoundCategory category, float volume) override;
    void SetCategoryMute(SoundCategory category, bool mute) override;
    void Update() override;

 private:
    std::shared_ptr<Engine::Audio::IAudioModule> module_;
    float sfx_volume_ = 1.0f;
    float music_volume_ = 1.0f;
};

}  // namespace Rtype::Client::Audio
