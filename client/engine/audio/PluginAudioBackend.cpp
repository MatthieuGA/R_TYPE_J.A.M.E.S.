/**
 * @file PluginAudioBackend.cpp
 * @brief Implementation of the plugin audio backend adapter.
 */

#include "engine/audio/PluginAudioBackend.hpp"

#include <memory>
#include <string>

namespace Rtype::Client::Audio {

PluginAudioBackend::PluginAudioBackend(
    std::shared_ptr<Engine::Audio::IAudioModule> module)
    : module_(module) {
    if (module_) {
        module_->Initialize();
    }
}

PluginAudioBackend::~PluginAudioBackend() {
    if (module_) {
        module_->Shutdown();
    }
}

bool PluginAudioBackend::LoadSound(
    const std::string &id, const std::string &path) {
    if (!module_) {
        return false;
    }
    return module_->LoadSound(id, path);
}

bool PluginAudioBackend::LoadMusic(
    const std::string &id, const std::string &path) {
    if (!module_) {
        return false;
    }
    return module_->LoadMusic(id, path);
}

void PluginAudioBackend::Play(const PlaybackRequest &request) {
    if (!module_) {
        return;
    }

    // Convert PlaybackRequest to plugin PlaybackRequest
    Engine::Audio::PlaybackRequest plugin_request;
    plugin_request.id = request.id;
    plugin_request.volume = request.volume;
    plugin_request.loop = request.loop;

    // Convert category
    if (request.category == SoundCategory::MUSIC) {
        plugin_request.category =
            Engine::Audio::PlaybackRequest::Category::MUSIC;
    } else {
        plugin_request.category =
            Engine::Audio::PlaybackRequest::Category::SFX;
    }

    module_->Play(plugin_request);
}

void PluginAudioBackend::StopMusic() {
    if (module_) {
        module_->StopMusic();
    }
}

void PluginAudioBackend::SetCategoryVolume(
    SoundCategory category, float volume) {
    if (!module_) {
        return;
    }

    if (category == SoundCategory::MUSIC) {
        music_volume_ = volume;
        module_->SetMusicVolume(volume);
    } else {
        sfx_volume_ = volume;
        module_->SetSfxVolume(volume);
    }
}

void PluginAudioBackend::SetCategoryMute(SoundCategory category, bool mute) {
    if (!module_) {
        return;
    }

    if (category == SoundCategory::MUSIC) {
        module_->MuteMusic(mute);
    } else {
        module_->MuteSfx(mute);
    }
}

void PluginAudioBackend::Update() {
    if (module_) {
        module_->Update(0.016f);  // ~60fps default delta
    }
}

}  // namespace Rtype::Client::Audio
