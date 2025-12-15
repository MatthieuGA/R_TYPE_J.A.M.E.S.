/**
 * @file SFMLAudioModule.hpp
 * @brief SFML-based implementation of IAudioModule.
 */

#pragma once
#include <map>
#include <memory>
#include <string>

#include <SFML/Audio.hpp>
#include <audio/IAudioModule.hpp>

namespace Engine {
namespace Audio {

/**
 * @brief SFML implementation of the audio module interface.
 *
 * This module will be compiled as a .so plugin and loaded dynamically.
 */
class SFMLAudioModule : public IAudioModule {
 public:
    SFMLAudioModule() = default;
    ~SFMLAudioModule() override = default;

    bool Initialize() override;
    void Shutdown() override;
    void Update(float delta_time) override;

    bool LoadSound(const std::string &id, const std::string &path) override;
    bool LoadMusic(const std::string &id, const std::string &path) override;
    void Play(const PlaybackRequest &request) override;
    void StopMusic() override;

    void SetSfxVolume(float volume) override;
    void SetMusicVolume(float volume) override;
    void MuteSfx(bool mute) override;
    void MuteMusic(bool mute) override;

    std::string GetModuleName() const override;

 private:
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;
    std::map<std::string, std::unique_ptr<sf::Sound>> sounds_;
    std::map<std::string, std::unique_ptr<sf::Music>> music_tracks_;

    sf::Music *current_music_ = nullptr;

    float sfx_volume_ = 100.0f;
    float music_volume_ = 100.0f;
    bool sfx_muted_ = false;
    bool music_muted_ = false;
};

}  // namespace Audio
}  // namespace Engine

// C-style entry point for dynamic loading
extern "C" {
std::shared_ptr<Engine::Audio::IAudioModule> entryPoint();
}
