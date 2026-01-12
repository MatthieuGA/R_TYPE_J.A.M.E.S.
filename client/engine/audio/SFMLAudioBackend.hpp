/**
 * @file SFMLAudioBackend.hpp
 * @brief SFML-based implementation of IAudioBackend.
 */

#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Audio.hpp>

#include "include/audio/IAudioBackend.hpp"

namespace Rtype::Client::Audio {

/**
 * @brief SFML implementation of the audio backend.
 *
 * This is the ONLY class allowed to use SFML Audio directly.
 * Internally uses a thread-safe queue for non-blocking playback.
 */
class SFMLAudioBackend : public IAudioBackend {
 public:
    SFMLAudioBackend();
    ~SFMLAudioBackend() override;

    bool LoadSound(const std::string &id, const std::string &path) override;
    bool LoadMusic(const std::string &id, const std::string &path) override;
    void Play(const PlaybackRequest &request) override;
    void StopMusic() override;
    void SetCategoryVolume(SoundCategory category, float volume) override;
    float GetCategoryVolume(SoundCategory category) const override;
    bool GetCategoryMuteStatus(SoundCategory category) const override;
    void SetCategoryMute(SoundCategory category, bool mute) override;
    void Update() override;

 private:
    struct SoundInstance {
        sf::Sound sound;
        bool in_use = false;
    };

    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>>
        sound_buffers_;
    std::vector<SoundInstance> sound_pool_;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> music_map_;

    std::string current_music_id_;
    float sfx_volume_ = 1.0f;
    float music_volume_ = 1.0f;
    bool sfx_muted_ = false;
    bool music_muted_ = false;

    std::queue<PlaybackRequest> playback_queue_;
    std::mutex queue_mutex_;

    static constexpr size_t kMaxConcurrentSounds = 16;

    void ProcessPlaybackQueue();
    void PlaySoundImmediate(const PlaybackRequest &request);
    void PlayMusicImmediate(const PlaybackRequest &request);
    SoundInstance *GetAvailableSoundInstance();
    float GetEffectiveVolume(SoundCategory category, float request_volume);
};

}  // namespace Rtype::Client::Audio
