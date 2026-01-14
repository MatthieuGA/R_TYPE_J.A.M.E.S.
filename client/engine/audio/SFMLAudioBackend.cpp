/**
 * @file SFMLAudioBackend.cpp
 * @brief Implementation of SFML audio backend.
 */

#include "engine/audio/SFMLAudioBackend.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace Rtype::Client::Audio {

SFMLAudioBackend::SFMLAudioBackend() {
    sound_pool_.resize(kMaxConcurrentSounds);
}

SFMLAudioBackend::~SFMLAudioBackend() {
    StopMusic();
}

bool SFMLAudioBackend::LoadSound(
    const std::string &id, const std::string &path) {
    // Skip if already loaded
    if (sound_buffers_.find(id) != sound_buffers_.end()) {
        return true;
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(path)) {
        std::cerr << "[Audio] Failed to load sound: " << path << std::endl;
        return false;
    }
    sound_buffers_[id] = std::move(buffer);
    return true;
}

bool SFMLAudioBackend::LoadMusic(
    const std::string &id, const std::string &path) {
    // Skip if already loaded
    if (music_map_.find(id) != music_map_.end()) {
        return true;
    }

    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(path)) {
        std::cerr << "[Audio] Failed to load music: " << path << std::endl;
        return false;
    }
    music_map_[id] = std::move(music);
    return true;
}

void SFMLAudioBackend::Play(const PlaybackRequest &request) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    playback_queue_.push(request);
}

void SFMLAudioBackend::StopMusic() {
    for (auto &[id, music] : music_map_) {
        if (music->getStatus() == sf::Music::Playing) {
            music->stop();
        }
    }
    current_music_id_.clear();
}

bool SFMLAudioBackend::IsMusicPlaying(const std::string &id) const {
    auto it = music_map_.find(id);
    if (it == music_map_.end()) {
        return false;
    }
    return it->second->getStatus() == sf::Music::Playing;
}

void SFMLAudioBackend::SetCategoryVolume(
    SoundCategory category, float volume) {
    volume = std::max(0.0f, std::min(1.0f, volume));
    if (category == SoundCategory::SFX) {
        sfx_volume_ = volume;
    } else {
        music_volume_ = volume;
        for (auto &[id, music] : music_map_) {
            if (music->getStatus() == sf::Music::Playing) {
                music->setVolume(
                    GetEffectiveVolume(SoundCategory::MUSIC, 1.0f) * 100.0f);
            }
        }
    }
}

void SFMLAudioBackend::SetCategoryMute(SoundCategory category, bool mute) {
    if (category == SoundCategory::SFX) {
        sfx_muted_ = mute;
    } else {
        music_muted_ = mute;
        for (auto &[id, music] : music_map_) {
            if (music->getStatus() == sf::Music::Playing) {
                music->setVolume(
                    GetEffectiveVolume(SoundCategory::MUSIC, 1.0f) * 100.0f);
            }
        }
    }
}

void SFMLAudioBackend::Update() {
    // Clean up finished sounds
    for (auto &instance : sound_pool_) {
        if (instance.in_use &&
            instance.sound.getStatus() == sf::Sound::Stopped) {
            instance.in_use = false;
        }
    }

    // Process queued playback requests
    ProcessPlaybackQueue();
}

void SFMLAudioBackend::ProcessPlaybackQueue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!playback_queue_.empty()) {
        const auto &request = playback_queue_.front();
        if (request.category == SoundCategory::SFX) {
            PlaySoundImmediate(request);
        } else {
            PlayMusicImmediate(request);
        }
        playback_queue_.pop();
    }
}

void SFMLAudioBackend::PlaySoundImmediate(const PlaybackRequest &request) {
    auto it = sound_buffers_.find(request.id);
    if (it == sound_buffers_.end()) {
        std::cerr << "[Audio] Sound not found: " << request.id << std::endl;
        return;
    }

    SoundInstance *instance = GetAvailableSoundInstance();
    if (!instance) {
        // std::cerr << "[Audio] No available sound instance" << std::endl;
        // Commented because it spams the console if too many sounds are played
        return;
    }

    instance->sound.setBuffer(*it->second);
    instance->sound.setVolume(
        GetEffectiveVolume(SoundCategory::SFX, request.volume) * 100.0f);
    instance->sound.setLoop(request.loop);
    instance->sound.play();
    instance->in_use = true;
}

void SFMLAudioBackend::PlayMusicImmediate(const PlaybackRequest &request) {
    auto it = music_map_.find(request.id);
    if (it == music_map_.end()) {
        std::cerr << "[Audio] Music not found: " << request.id << std::endl;
        return;
    }

    // Stop current music if different
    if (current_music_id_ != request.id) {
        StopMusic();
        current_music_id_ = request.id;
    }

    it->second->setVolume(
        GetEffectiveVolume(SoundCategory::MUSIC, request.volume) * 100.0f);
    it->second->setLoop(request.loop);
    it->second->play();
}

SFMLAudioBackend::SoundInstance *
SFMLAudioBackend::GetAvailableSoundInstance() {
    for (auto &instance : sound_pool_) {
        if (!instance.in_use) {
            return &instance;
        }
    }
    return nullptr;
}

float SFMLAudioBackend::GetEffectiveVolume(
    SoundCategory category, float request_volume) {
    if (category == SoundCategory::SFX) {
        return sfx_muted_ ? 0.0f : (sfx_volume_ * request_volume);
    } else {
        return music_muted_ ? 0.0f : (music_volume_ * request_volume);
    }
}

float SFMLAudioBackend::GetCategoryVolume(SoundCategory category) const {
    if (category == SoundCategory::SFX) {
        return sfx_volume_;
    } else {
        return music_volume_;
    }
}

bool SFMLAudioBackend::GetCategoryMuteStatus(SoundCategory category) const {
    if (category == SoundCategory::SFX) {
        return sfx_muted_;
    } else {
        return music_muted_;
    }
}

}  // namespace Rtype::Client::Audio
