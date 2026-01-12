/**
 * @file test_audio_mock.cpp
 * @brief Standalone unit tests for audio subsystem (no SFML/OpenAL
 * dependencies).
 *
 * These tests use only the mock backend and don't require linking against
 * the actual audio libraries, avoiding dependency issues.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "include/audio/AudioTypes.hpp"
#include "include/audio/IAudioBackend.hpp"

namespace Rtype::Client::Audio {

// Note: Using the real AudioManager implementation from
// engine/audio/AudioManager.hpp instead of duplicating it here. This ensures
// tests stay synchronized with production code changes.

/**
 * @brief Mock audio backend for testing.
 */
class MockAudioBackend : public IAudioBackend {
 public:
    struct LoadSoundCall {
        std::string id;
        std::string path;
    };

    struct LoadMusicCall {
        std::string id;
        std::string path;
    };

    struct PlayCall {
        std::string id;
        float volume;
        bool loop;
        SoundCategory category;
    };

    std::vector<LoadSoundCall> load_sound_calls_;
    std::vector<LoadMusicCall> load_music_calls_;
    std::vector<PlayCall> play_calls_;
    int stop_music_calls_ = 0;
    int update_calls_ = 0;
    float sfx_volume_ = 1.0f;
    float music_volume_ = 1.0f;
    bool sfx_muted_ = false;
    bool music_muted_ = false;
    bool load_sound_return_ = true;
    bool load_music_return_ = true;

    bool LoadSound(const std::string &id, const std::string &path) override {
        load_sound_calls_.push_back({id, path});
        return load_sound_return_;
    }

    bool LoadMusic(const std::string &id, const std::string &path) override {
        load_music_calls_.push_back({id, path});
        return load_music_return_;
    }

    void Play(const PlaybackRequest &request) override {
        play_calls_.push_back(
            {request.id, request.volume, request.loop, request.category});
    }

    void StopMusic() override {
        stop_music_calls_++;
    }

    bool IsMusicPlaying(const std::string &id) const override {
        // Simple mock: return false (can be extended if needed)
        return false;
    }

    void SetCategoryVolume(SoundCategory category, float volume) override {
        if (category == SoundCategory::SFX) {
            sfx_volume_ = volume;
        } else {
            music_volume_ = volume;
        }
    }

    float GetCategoryVolume(SoundCategory category) const override {
        if (category == SoundCategory::SFX) {
            return sfx_volume_;
        } else {
            return music_volume_;
        }
    }

    void SetCategoryMute(SoundCategory category, bool mute) override {
        if (category == SoundCategory::SFX) {
            sfx_muted_ = mute;
        } else {
            music_muted_ = mute;
        }
    }

    bool GetCategoryMuteStatus(SoundCategory category) const override {
        if (category == SoundCategory::SFX) {
            return sfx_muted_;
        } else {
            return music_muted_;
        }
    }

    void Update() override {
        update_calls_++;
    }
};

}  // namespace Rtype::Client::Audio

// Test Fixtures
class AudioManagerStandaloneTest : public ::testing::Test {
 protected:
    void SetUp() override {
        auto backend =
            std::make_unique<Rtype::Client::Audio::MockAudioBackend>();
        mock_backend_ = backend.get();
        audio_manager_ = std::make_unique<Rtype::Client::Audio::AudioManager>(
            std::move(backend));
    }

    void TearDown() override {
        audio_manager_.reset();
        mock_backend_ = nullptr;
    }

    Rtype::Client::Audio::MockAudioBackend *mock_backend_;
    std::unique_ptr<Rtype::Client::Audio::AudioManager> audio_manager_;
};

// ============================================================================
// AudioManager Tests
// ============================================================================

TEST_F(AudioManagerStandaloneTest, RegisterSoundAsset) {
    bool result =
        audio_manager_->RegisterAsset("test_sound", "test.wav", false);

    EXPECT_TRUE(result);
    ASSERT_EQ(mock_backend_->load_sound_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_sound_calls_[0].id, "test_sound");
    EXPECT_EQ(mock_backend_->load_sound_calls_[0].path, "test.wav");
    EXPECT_EQ(mock_backend_->load_music_calls_.size(), 0);
}

TEST_F(AudioManagerStandaloneTest, RegisterMusicAsset) {
    bool result =
        audio_manager_->RegisterAsset("test_music", "music.ogg", true);

    EXPECT_TRUE(result);
    ASSERT_EQ(mock_backend_->load_music_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_music_calls_[0].id, "test_music");
    EXPECT_EQ(mock_backend_->load_music_calls_[0].path, "music.ogg");
    EXPECT_EQ(mock_backend_->load_sound_calls_.size(), 0);
}

TEST_F(AudioManagerStandaloneTest, PlaySoundWithDefaultVolume) {
    audio_manager_->PlaySound("explosion");

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "explosion");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 1.0f);
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::SFX);
}

TEST_F(AudioManagerStandaloneTest, PlaySoundWithCustomVolume) {
    audio_manager_->PlaySound("laser", 0.5f);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "laser");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 0.5f);
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
}

TEST_F(AudioManagerStandaloneTest, PlayMusicWithLoop) {
    audio_manager_->PlayMusic("bgm", true);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "bgm");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 1.0f);
    EXPECT_TRUE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::MUSIC);
}

TEST_F(AudioManagerStandaloneTest, PlayMusicWithoutLoop) {
    audio_manager_->PlayMusic("jingle", false);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "jingle");
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
}

TEST_F(AudioManagerStandaloneTest, StopMusic) {
    audio_manager_->StopMusic();
    EXPECT_EQ(mock_backend_->stop_music_calls_, 1);
}

TEST_F(AudioManagerStandaloneTest, SetSfxVolume) {
    audio_manager_->SetSfxVolume(0.7f);
    EXPECT_FLOAT_EQ(mock_backend_->sfx_volume_, 0.7f);
}

TEST_F(AudioManagerStandaloneTest, SetMusicVolume) {
    audio_manager_->SetMusicVolume(0.4f);
    EXPECT_FLOAT_EQ(mock_backend_->music_volume_, 0.4f);
}

TEST_F(AudioManagerStandaloneTest, MuteSfx) {
    audio_manager_->MuteSfx(true);
    EXPECT_TRUE(mock_backend_->sfx_muted_);
}

TEST_F(AudioManagerStandaloneTest, UnmuteSfx) {
    audio_manager_->MuteSfx(true);
    audio_manager_->MuteSfx(false);
    EXPECT_FALSE(mock_backend_->sfx_muted_);
}

TEST_F(AudioManagerStandaloneTest, MuteMusic) {
    audio_manager_->MuteMusic(true);
    EXPECT_TRUE(mock_backend_->music_muted_);
}

TEST_F(AudioManagerStandaloneTest, UnmuteMusic) {
    audio_manager_->MuteMusic(true);
    audio_manager_->MuteMusic(false);
    EXPECT_FALSE(mock_backend_->music_muted_);
}

TEST_F(AudioManagerStandaloneTest, Update) {
    audio_manager_->Update();
    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

TEST_F(AudioManagerStandaloneTest, MultipleOperations) {
    audio_manager_->RegisterAsset("sound1", "s1.wav", false);
    audio_manager_->RegisterAsset("music1", "m1.ogg", true);
    audio_manager_->PlaySound("sound1", 0.8f);
    audio_manager_->PlaySound("sound1", 0.6f);
    audio_manager_->PlayMusic("music1", true);
    audio_manager_->Update();

    EXPECT_EQ(mock_backend_->load_sound_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_music_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_.size(), 3);
    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

TEST_F(AudioManagerStandaloneTest, RegisterAssetFailure) {
    mock_backend_->load_sound_return_ = false;
    bool result =
        audio_manager_->RegisterAsset("bad_sound", "missing.wav", false);
    EXPECT_FALSE(result);
}

TEST_F(AudioManagerStandaloneTest, VolumeEdgeCases) {
    audio_manager_->SetSfxVolume(0.0f);
    EXPECT_FLOAT_EQ(mock_backend_->sfx_volume_, 0.0f);

    audio_manager_->SetMusicVolume(1.0f);
    EXPECT_FLOAT_EQ(mock_backend_->music_volume_, 1.0f);
}

TEST_F(AudioManagerStandaloneTest, EmptyStringId) {
    EXPECT_NO_THROW(audio_manager_->PlaySound(""));
    EXPECT_EQ(mock_backend_->play_calls_.size(), 1);
}

TEST_F(AudioManagerStandaloneTest, ZeroVolume) {
    audio_manager_->PlaySound("silent", 0.0f);
    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 0.0f);
}

// ============================================================================
// AudioTypes Tests
// ============================================================================

TEST(AudioTypesStandaloneTest, PlaybackRequestCopyable) {
    Rtype::Client::Audio::PlaybackRequest req1;
    req1.id = "test";
    req1.volume = 0.5f;
    req1.loop = true;

    Rtype::Client::Audio::PlaybackRequest req2 = req1;

    EXPECT_EQ(req2.id, "test");
    EXPECT_FLOAT_EQ(req2.volume, 0.5f);
    EXPECT_TRUE(req2.loop);
}

// ============================================================================
// Interface Tests
// ============================================================================

TEST(IAudioBackendTest, MockBackendImplementsInterface) {
    std::unique_ptr<Rtype::Client::Audio::IAudioBackend> backend =
        std::make_unique<Rtype::Client::Audio::MockAudioBackend>();

    EXPECT_TRUE(backend->LoadSound("test", "test.wav"));
    EXPECT_TRUE(backend->LoadMusic("music", "music.ogg"));
    EXPECT_NO_THROW(backend->StopMusic());
    EXPECT_NO_THROW(backend->Update());
}
