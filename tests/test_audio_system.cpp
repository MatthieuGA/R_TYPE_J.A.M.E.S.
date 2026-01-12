/**
 * @file test_audio_system.cpp
 * @brief Unit tests for the audio subsystem.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/audio/AudioTypes.hpp"
#include "include/audio/IAudioBackend.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/registry.hpp"

namespace Rtype::Client::Audio {

/**
 * @brief Mock audio backend for testing.
 *
 * Records all operations without actually playing audio.
 */
class MockAudioBackend : public IAudioBackend {
 public:
    // Recording structures
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

    // Recorded calls
    std::vector<LoadSoundCall> load_sound_calls_;
    std::vector<LoadMusicCall> load_music_calls_;
    std::vector<PlayCall> play_calls_;
    int stop_music_calls_ = 0;
    int update_calls_ = 0;

    // Volume and mute state
    float sfx_volume_ = 1.0f;
    float music_volume_ = 1.0f;
    bool sfx_muted_ = false;
    bool music_muted_ = false;

    // Control return values
    bool load_sound_return_ = true;
    bool load_music_return_ = true;

    // IAudioBackend implementation
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

    // Helper methods for testing
    void Reset() {
        load_sound_calls_.clear();
        load_music_calls_.clear();
        play_calls_.clear();
        stop_music_calls_ = 0;
        update_calls_ = 0;
        sfx_volume_ = 1.0f;
        music_volume_ = 1.0f;
        sfx_muted_ = false;
        music_muted_ = false;
    }
};

}  // namespace Rtype::Client::Audio

// Test Fixtures
class AudioManagerTest : public ::testing::Test {
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

class AudioSystemTest : public ::testing::Test {
 protected:
    void SetUp() override {
        registry_.RegisterComponent<Rtype::Client::Component::SoundRequest>();

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

    Engine::registry registry_;
    Rtype::Client::Audio::MockAudioBackend *mock_backend_;
    std::unique_ptr<Rtype::Client::Audio::AudioManager> audio_manager_;
};

// ============================================================================
// AudioManager Tests
// ============================================================================

TEST_F(AudioManagerTest, RegisterSoundAsset) {
    bool result =
        audio_manager_->RegisterAsset("test_sound", "test.wav", false);

    EXPECT_TRUE(result);
    ASSERT_EQ(mock_backend_->load_sound_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_sound_calls_[0].id, "test_sound");
    EXPECT_EQ(mock_backend_->load_sound_calls_[0].path, "test.wav");
    EXPECT_EQ(mock_backend_->load_music_calls_.size(), 0);
}

TEST_F(AudioManagerTest, RegisterMusicAsset) {
    bool result =
        audio_manager_->RegisterAsset("test_music", "music.ogg", true);

    EXPECT_TRUE(result);
    ASSERT_EQ(mock_backend_->load_music_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_music_calls_[0].id, "test_music");
    EXPECT_EQ(mock_backend_->load_music_calls_[0].path, "music.ogg");
    EXPECT_EQ(mock_backend_->load_sound_calls_.size(), 0);
}

TEST_F(AudioManagerTest, PlaySoundWithDefaultVolume) {
    audio_manager_->PlaySound("explosion");

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "explosion");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 1.0f);
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::SFX);
}

TEST_F(AudioManagerTest, PlaySoundWithCustomVolume) {
    audio_manager_->PlaySound("laser", 0.5f);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "laser");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 0.5f);
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::SFX);
}

TEST_F(AudioManagerTest, PlayMusicWithLoop) {
    audio_manager_->PlayMusic("bgm", true);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "bgm");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 1.0f);
    EXPECT_TRUE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::MUSIC);
}

TEST_F(AudioManagerTest, PlayMusicWithoutLoop) {
    audio_manager_->PlayMusic("jingle", false);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "jingle");
    EXPECT_FALSE(mock_backend_->play_calls_[0].loop);
    EXPECT_EQ(mock_backend_->play_calls_[0].category,
        Rtype::Client::Audio::SoundCategory::MUSIC);
}

TEST_F(AudioManagerTest, StopMusic) {
    audio_manager_->StopMusic();

    EXPECT_EQ(mock_backend_->stop_music_calls_, 1);
}

TEST_F(AudioManagerTest, SetSfxVolume) {
    audio_manager_->SetSfxVolume(0.7f);

    EXPECT_FLOAT_EQ(mock_backend_->sfx_volume_, 0.7f);
}

TEST_F(AudioManagerTest, SetMusicVolume) {
    audio_manager_->SetMusicVolume(0.4f);

    EXPECT_FLOAT_EQ(mock_backend_->music_volume_, 0.4f);
}

TEST_F(AudioManagerTest, MuteSfx) {
    audio_manager_->MuteSfx(true);

    EXPECT_TRUE(mock_backend_->sfx_muted_);
}

TEST_F(AudioManagerTest, UnmuteSfx) {
    audio_manager_->MuteSfx(true);
    audio_manager_->MuteSfx(false);

    EXPECT_FALSE(mock_backend_->sfx_muted_);
}

TEST_F(AudioManagerTest, MuteMusic) {
    audio_manager_->MuteMusic(true);

    EXPECT_TRUE(mock_backend_->music_muted_);
}

TEST_F(AudioManagerTest, UnmuteMusic) {
    audio_manager_->MuteMusic(true);
    audio_manager_->MuteMusic(false);

    EXPECT_FALSE(mock_backend_->music_muted_);
}

TEST_F(AudioManagerTest, Update) {
    audio_manager_->Update();

    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

TEST_F(AudioManagerTest, MultipleOperations) {
    // Register assets
    audio_manager_->RegisterAsset("sound1", "s1.wav", false);
    audio_manager_->RegisterAsset("music1", "m1.ogg", true);

    // Play multiple sounds
    audio_manager_->PlaySound("sound1", 0.8f);
    audio_manager_->PlaySound("sound1", 0.6f);
    audio_manager_->PlayMusic("music1", true);

    // Update
    audio_manager_->Update();

    EXPECT_EQ(mock_backend_->load_sound_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->load_music_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_.size(), 3);
    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

// ============================================================================
// AudioSystem Integration Tests
// ============================================================================

TEST_F(AudioSystemTest, ProcessSingleSoundRequest) {
    // Create entity with SoundRequest
    auto entity = registry_.SpawnEntity();
    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity, Rtype::Client::Component::SoundRequest{
                    .sound_id = "test_sound", .volume = 0.9f, .loop = false});

    // Verify component exists
    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    size_t entity_index = entity.GetId();
    ASSERT_TRUE(sound_requests[entity_index].has_value());

    // Run audio system
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    // Verify sound was played
    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_EQ(mock_backend_->play_calls_[0].id, "test_sound");
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 0.9f);

    // Verify component was removed
    EXPECT_FALSE(sound_requests[entity_index].has_value());

    // Verify Update was called
    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

TEST_F(AudioSystemTest, ProcessMultipleSoundRequests) {
    // Create multiple entities with SoundRequests
    auto entity1 = registry_.SpawnEntity();
    auto entity2 = registry_.SpawnEntity();
    auto entity3 = registry_.SpawnEntity();

    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(entity1,
        Rtype::Client::Component::SoundRequest{"sound1", 1.0f, false});

    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(entity2,
        Rtype::Client::Component::SoundRequest{"sound2", 0.5f, false});

    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity3, Rtype::Client::Component::SoundRequest{"sound3", 0.7f, true});

    // Run audio system
    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    // Verify all sounds were played
    ASSERT_EQ(mock_backend_->play_calls_.size(), 3);

    // Verify all components were removed
    size_t idx1 = entity1.GetId();
    size_t idx2 = entity2.GetId();
    size_t idx3 = entity3.GetId();
    EXPECT_FALSE(sound_requests[idx1].has_value());
    EXPECT_FALSE(sound_requests[idx2].has_value());
    EXPECT_FALSE(sound_requests[idx3].has_value());
}

TEST_F(AudioSystemTest, ProcessNoSoundRequests) {
    // Run audio system with no entities
    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    // Verify no sounds were played
    EXPECT_EQ(mock_backend_->play_calls_.size(), 0);

    // Verify Update was still called
    EXPECT_EQ(mock_backend_->update_calls_, 1);
}

TEST_F(AudioSystemTest, EntityWithoutSoundRequestNotAffected) {
    // Create entity without SoundRequest
    auto entity1 = registry_.SpawnEntity();

    // Create entity with SoundRequest
    auto entity2 = registry_.SpawnEntity();
    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity2, Rtype::Client::Component::SoundRequest{"sound", 1.0f, false});

    // Run audio system
    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    // Verify only one sound played
    EXPECT_EQ(mock_backend_->play_calls_.size(), 1);

    // Verify entity1 still exists (not killed)
    // Note: In the actual implementation, entities themselves are not killed,
    // only the SoundRequest component is removed
}

TEST_F(AudioSystemTest, SoundRequestWithDifferentVolumes) {
    auto entity1 = registry_.SpawnEntity();
    auto entity2 = registry_.SpawnEntity();

    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity1, Rtype::Client::Component::SoundRequest{"quiet", 0.1f, false});

    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity2, Rtype::Client::Component::SoundRequest{"loud", 1.0f, false});

    // Run audio system
    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    // Verify both sounds played with correct volumes
    ASSERT_EQ(mock_backend_->play_calls_.size(), 2);

    // Find the calls (order may vary)
    bool found_quiet = false;
    bool found_loud = false;

    for (const auto &call : mock_backend_->play_calls_) {
        if (call.id == "quiet" && std::abs(call.volume - 0.1f) < 0.001f) {
            found_quiet = true;
        }
        if (call.id == "loud" && std::abs(call.volume - 1.0f) < 0.001f) {
            found_loud = true;
        }
    }

    EXPECT_TRUE(found_quiet);
    EXPECT_TRUE(found_loud);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(AudioManagerTest, RegisterAssetFailure) {
    mock_backend_->load_sound_return_ = false;

    bool result =
        audio_manager_->RegisterAsset("bad_sound", "missing.wav", false);

    EXPECT_FALSE(result);
}

TEST_F(AudioManagerTest, VolumeClampingEdgeCases) {
    // Test minimum volume
    audio_manager_->SetSfxVolume(0.0f);
    EXPECT_FLOAT_EQ(mock_backend_->sfx_volume_, 0.0f);

    // Test maximum volume
    audio_manager_->SetMusicVolume(1.0f);
    EXPECT_FLOAT_EQ(mock_backend_->music_volume_, 1.0f);
}

TEST_F(AudioSystemTest, EmptyStringId) {
    auto entity = registry_.SpawnEntity();
    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity, Rtype::Client::Component::SoundRequest{"", 1.0f, false});

    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();

    // Should not crash
    EXPECT_NO_THROW(Rtype::Client::AudioSystem(
        registry_, *audio_manager_, sound_requests));

    // Should still attempt to play
    EXPECT_EQ(mock_backend_->play_calls_.size(), 1);
}

TEST_F(AudioSystemTest, ZeroVolume) {
    auto entity = registry_.SpawnEntity();
    registry_.EmplaceComponent<Rtype::Client::Component::SoundRequest>(
        entity, Rtype::Client::Component::SoundRequest{"silent", 0.0f, false});

    auto &sound_requests =
        registry_.GetComponents<Rtype::Client::Component::SoundRequest>();
    Rtype::Client::AudioSystem(registry_, *audio_manager_, sound_requests);

    ASSERT_EQ(mock_backend_->play_calls_.size(), 1);
    EXPECT_FLOAT_EQ(mock_backend_->play_calls_[0].volume, 0.0f);
}

// ============================================================================
// AudioTypes Tests
// ============================================================================

TEST(AudioTypesTest, PlaybackRequestDefaultValues) {
    Rtype::Client::Audio::PlaybackRequest request;

    EXPECT_EQ(request.id, "");
    EXPECT_FLOAT_EQ(request.volume, 1.0f);
    EXPECT_FALSE(request.loop);
    EXPECT_EQ(request.category, Rtype::Client::Audio::SoundCategory::SFX);
}

TEST(AudioTypesTest, SoundCategoryEnum) {
    using Rtype::Client::Audio::SoundCategory;

    SoundCategory sfx = SoundCategory::SFX;
    SoundCategory music = SoundCategory::MUSIC;

    EXPECT_NE(sfx, music);
}

// ============================================================================
// Component Tests
// ============================================================================

TEST(SoundRequestComponentTest, DefaultValues) {
    Rtype::Client::Component::SoundRequest request;

    EXPECT_EQ(request.sound_id, "");
    EXPECT_FLOAT_EQ(request.volume, 1.0f);
    EXPECT_FALSE(request.loop);
}

TEST(SoundRequestComponentTest, CustomValues) {
    Rtype::Client::Component::SoundRequest request{
        .sound_id = "custom", .volume = 0.5f, .loop = true};

    EXPECT_EQ(request.sound_id, "custom");
    EXPECT_FLOAT_EQ(request.volume, 0.5f);
    EXPECT_TRUE(request.loop);
}

TEST(SoundRequestComponentTest, IsPOD) {
    // SoundRequest should be a POD-like struct (though std::string makes it
    // non-trivial) We can verify it's movable and copyable
    Rtype::Client::Component::SoundRequest req1{"test", 1.0f, false};
    Rtype::Client::Component::SoundRequest req2 = req1;

    EXPECT_EQ(req2.sound_id, "test");
    EXPECT_FLOAT_EQ(req2.volume, 1.0f);
    EXPECT_FALSE(req2.loop);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
