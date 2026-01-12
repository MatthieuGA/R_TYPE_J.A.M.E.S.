/**
 * @file AudioManager.cpp
 * @brief Implementation of high-level audio manager.
 */

#include "engine/audio/AudioManager.hpp"

#include <string>
#include <utility>

namespace Rtype::Client::Audio {

AudioManager::AudioManager(std::unique_ptr<IAudioBackend> backend)
    : backend_(std::move(backend)) {}

AudioManager::~AudioManager() = default;

bool AudioManager::RegisterAsset(
    const std::string &id, const std::string &path, bool is_music) {
    if (is_music) {
        return backend_->LoadMusic(id, path);
    } else {
        return backend_->LoadSound(id, path);
    }
}

void AudioManager::PlaySound(const std::string &id, float volume) {
    PlaybackRequest request;
    request.id = id;
    request.volume = volume;
    request.loop = false;
    request.category = SoundCategory::SFX;
    backend_->Play(request);
}

void AudioManager::PlayMusic(const std::string &id, bool loop) {
    PlaybackRequest request;
    request.id = id;
    request.volume = 1.0f;
    request.loop = loop;
    request.category = SoundCategory::MUSIC;
    backend_->Play(request);
}

void AudioManager::StopMusic() {
    backend_->StopMusic();
}

void AudioManager::SetSfxVolume(float volume) {
    backend_->SetCategoryVolume(SoundCategory::SFX, volume);
}

void AudioManager::SetMusicVolume(float volume) {
    backend_->SetCategoryVolume(SoundCategory::MUSIC, volume);
}

void AudioManager::MuteSfx(bool mute) {
    backend_->SetCategoryMute(SoundCategory::SFX, mute);
}

void AudioManager::MuteMusic(bool mute) {
    backend_->SetCategoryMute(SoundCategory::MUSIC, mute);
}

void AudioManager::Update() {
    backend_->Update();
}

float AudioManager::GetSfxVolume() const {
    return backend_->GetCategoryVolume(SoundCategory::SFX);
}

bool AudioManager::IsSfxMuted() const {
    return backend_->GetCategoryMuteStatus(SoundCategory::SFX);
}

float AudioManager::GetMusicVolume() const {
    return backend_->GetCategoryVolume(SoundCategory::MUSIC);
}

bool AudioManager::IsMusicMuted() const {
    return backend_->GetCategoryMuteStatus(SoundCategory::MUSIC);
}

}  // namespace Rtype::Client::Audio
