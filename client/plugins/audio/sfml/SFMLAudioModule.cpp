/**
 * @file SFMLAudioModule.cpp
 * @brief Implementation of the SFML audio module plugin.
 */

#include "SFMLAudioModule.hpp"  // NOLINT(build/include_subdir)

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace Engine {
namespace Audio {

bool SFMLAudioModule::Initialize() {
    std::cout << "[SFMLAudioModule] Initialized successfully" << std::endl;
    return true;
}

void SFMLAudioModule::Shutdown() {
    sounds_.clear();
    sound_buffers_.clear();
    music_tracks_.clear();
    current_music_ = nullptr;
    std::cout << "[SFMLAudioModule] Shut down" << std::endl;
}

void SFMLAudioModule::Update(float delta_time) {
    // Update music streaming if needed
    // SFML handles this automatically
}

bool SFMLAudioModule::LoadSound(
    const std::string &id, const std::string &path) {
    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(path)) {
        std::cerr << "[SFMLAudioModule] Failed to load sound: " << path
                  << std::endl;
        return false;
    }

    sound_buffers_[id] = std::move(buffer);
    sounds_[id] = std::make_unique<sf::Sound>(*sound_buffers_[id]);

    std::cout << "[SFMLAudioModule] Loaded sound: " << id << " from " << path
              << std::endl;
    return true;
}

bool SFMLAudioModule::LoadMusic(
    const std::string &id, const std::string &path) {
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(path)) {
        std::cerr << "[SFMLAudioModule] Failed to load music: " << path
                  << std::endl;
        return false;
    }

    music_tracks_[id] = std::move(music);
    std::cout << "[SFMLAudioModule] Loaded music: " << id << " from " << path
              << std::endl;
    return true;
}

void SFMLAudioModule::Play(const PlaybackRequest &request) {
    if (request.category == PlaybackRequest::Category::MUSIC) {
        auto it = music_tracks_.find(request.id);
        if (it == music_tracks_.end()) {
            std::cerr << "[SFMLAudioModule] Music not found: " << request.id
                      << std::endl;
            return;
        }

        if (current_music_) {
            current_music_->stop();
        }

        current_music_ = it->second.get();
        current_music_->setLoop(request.loop);
        current_music_->setVolume(
            request.volume * music_volume_ * (music_muted_ ? 0.0f : 1.0f));
        current_music_->play();
    } else {
        auto it = sounds_.find(request.id);
        if (it == sounds_.end()) {
            std::cerr << "[SFMLAudioModule] Sound not found: " << request.id
                      << std::endl;
            return;
        }

        it->second->setLoop(request.loop);
        it->second->setVolume(
            request.volume * sfx_volume_ * (sfx_muted_ ? 0.0f : 1.0f));
        it->second->play();
    }
}

void SFMLAudioModule::StopMusic() {
    if (current_music_) {
        current_music_->stop();
        current_music_ = nullptr;
    }
}

void SFMLAudioModule::SetSfxVolume(float volume) {
    sfx_volume_ = volume * 100.0f;

    // Update all active sounds
    for (auto &[id, sound] : sounds_) {
        if (sound->getStatus() == sf::Sound::Playing) {
            sound->setVolume(sfx_volume_ * (sfx_muted_ ? 0.0f : 1.0f));
        }
    }
}

void SFMLAudioModule::SetMusicVolume(float volume) {
    music_volume_ = volume * 100.0f;

    if (current_music_) {
        current_music_->setVolume(
            music_volume_ * (music_muted_ ? 0.0f : 1.0f));
    }
}

void SFMLAudioModule::MuteSfx(bool mute) {
    sfx_muted_ = mute;

    for (auto &[id, sound] : sounds_) {
        if (sound->getStatus() == sf::Sound::Playing) {
            sound->setVolume(sfx_volume_ * (mute ? 0.0f : 1.0f));
        }
    }
}

void SFMLAudioModule::MuteMusic(bool mute) {
    music_muted_ = mute;

    if (current_music_) {
        current_music_->setVolume(music_volume_ * (mute ? 0.0f : 1.0f));
    }
}

std::string SFMLAudioModule::GetModuleName() const {
    return "SFML Audio Module";
}

}  // namespace Audio
}  // namespace Engine

// C-style entry point
extern "C" {
std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
    return std::make_shared<Engine::Audio::SFMLAudioModule>();
}
}
