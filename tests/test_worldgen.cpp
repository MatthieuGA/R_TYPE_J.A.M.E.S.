/**
 * @file test_worldgen.cpp
 * @brief Unit tests for the WorldGen configuration system.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "server/worldgen/WorldGen.hpp"

namespace worldgen {
namespace testing {

/**
 * @brief Test fixture for WorldGen tests.
 */
class WorldGenTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Create temporary test directories
        test_dir_ = std::filesystem::temp_directory_path() / "worldgen_test";

        // Clean up any previous test artifacts first
        std::filesystem::remove_all(test_dir_);

        core_dir_ = test_dir_ / "core";
        user_dir_ = test_dir_ / "user";

        std::filesystem::create_directories(core_dir_);
        std::filesystem::create_directories(user_dir_);

        // Set up logging
        log_messages_.clear();
        loader_.SetLogCallback([this](LogLevel level, const std::string &msg) {
            log_messages_.push_back({level, msg});
        });
    }

    void TearDown() override {
        // Clean up test directories
        std::filesystem::remove_all(test_dir_);
    }

    /**
     * @brief Creates a test WGF file with given content.
     */
    void CreateWGFFile(const std::filesystem::path &dir,
        const std::string &filename, const std::string &content) {
        std::ofstream file(dir / filename);
        file << content;
        file.close();
    }

    /**
     * @brief Creates a valid minimal WGF JSON string.
     */
    std::string CreateMinimalWGF(
        const std::string &uuid, const std::string &name, float difficulty) {
        return R"({
            "uuid": ")" +
               uuid + R"(",
            "name": ")" +
               name + R"(",
            "difficulty": )" +
               std::to_string(difficulty) + R"(,
            "obstacles": []
        })";
    }

    std::filesystem::path test_dir_;
    std::filesystem::path core_dir_;
    std::filesystem::path user_dir_;
    WorldGenConfigLoader loader_;
    std::vector<std::pair<LogLevel, std::string>> log_messages_;
};

// ============================================================================
// WorldGenConfigLoader Tests
// ============================================================================

TEST_F(WorldGenTest, LoadEmptyDirectories) {
    // Should return false when no WGFs are found
    EXPECT_FALSE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));
    EXPECT_FALSE(loader_.HasWGFs());
    EXPECT_EQ(loader_.GetStatistics().total_files_scanned, 0);
}

TEST_F(WorldGenTest, LoadValidCoreWGF) {
    // Create a valid WGF file
    CreateWGFFile(core_dir_, "test.wgf.json",
        CreateMinimalWGF(
            "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d", "Test Frame", 3.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));
    EXPECT_TRUE(loader_.HasWGFs());

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.core_files_loaded, 1);
    EXPECT_EQ(stats.user_files_loaded, 0);
    EXPECT_EQ(stats.files_skipped, 0);

    // Verify WGF data
    auto *wgf = loader_.GetWGFByUUID("a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d");
    ASSERT_NE(wgf, nullptr);
    EXPECT_EQ(wgf->name, "Test Frame");
    EXPECT_FLOAT_EQ(wgf->difficulty, 3.0f);
    EXPECT_TRUE(wgf->is_core);
}

TEST_F(WorldGenTest, LoadValidUserWGF) {
    // Create both core and user WGFs
    CreateWGFFile(core_dir_, "core.wgf.json",
        CreateMinimalWGF(
            "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d", "Core Frame", 2.0f));
    CreateWGFFile(user_dir_, "user.wgf.json",
        CreateMinimalWGF(
            "b2c3d4e5-f6a7-4b8c-9d0e-1f2a3b4c5d6e", "User Frame", 4.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.core_files_loaded, 1);
    EXPECT_EQ(stats.user_files_loaded, 1);

    // Verify user WGF is not core
    auto *user_wgf =
        loader_.GetWGFByUUID("b2c3d4e5-f6a7-4b8c-9d0e-1f2a3b4c5d6e");
    ASSERT_NE(user_wgf, nullptr);
    EXPECT_FALSE(user_wgf->is_core);
}

TEST_F(WorldGenTest, DuplicateUUIDUserSkipped) {
    // Create core and user WGFs with same UUID
    const std::string uuid = "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d";
    CreateWGFFile(core_dir_, "core.wgf.json",
        CreateMinimalWGF(uuid, "Core Frame", 2.0f));
    CreateWGFFile(user_dir_, "user.wgf.json",
        CreateMinimalWGF(uuid, "User Frame (Duplicate)", 5.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.core_files_loaded, 1);
    EXPECT_EQ(stats.user_files_loaded, 0);
    EXPECT_EQ(stats.duplicate_uuids, 1);

    // Core frame should be loaded
    auto *wgf = loader_.GetWGFByUUID(uuid);
    ASSERT_NE(wgf, nullptr);
    EXPECT_EQ(wgf->name, "Core Frame");
}

TEST_F(WorldGenTest, InvalidJSONSkipped) {
    // Create invalid JSON file
    CreateWGFFile(core_dir_, "invalid.wgf.json", "{ invalid json }");
    CreateWGFFile(core_dir_, "valid.wgf.json",
        CreateMinimalWGF(
            "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d", "Valid Frame", 3.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.core_files_loaded, 1);
    EXPECT_EQ(stats.parse_errors, 1);
}

TEST_F(WorldGenTest, MissingRequiredFieldSkipped) {
    // Create WGF missing UUID
    CreateWGFFile(core_dir_, "no_uuid.wgf.json", R"({
        "name": "No UUID Frame",
        "difficulty": 3.0,
        "obstacles": []
    })");

    EXPECT_FALSE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.validation_errors, 1);
}

TEST_F(WorldGenTest, InvalidUUIDFormatSkipped) {
    // Create WGF with invalid UUID format
    CreateWGFFile(core_dir_, "bad_uuid.wgf.json", R"({
        "uuid": "not-a-valid-uuid",
        "name": "Bad UUID Frame",
        "difficulty": 3.0,
        "obstacles": []
    })");

    EXPECT_FALSE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    const auto &stats = loader_.GetStatistics();
    EXPECT_EQ(stats.validation_errors, 1);
}

TEST_F(WorldGenTest, GetUUIDListIsSorted) {
    // Create WGFs with different UUIDs (intentionally out of order)
    CreateWGFFile(core_dir_, "z_last.wgf.json",
        CreateMinimalWGF(
            "f0000000-0000-4000-8000-000000000000", "Last", 1.0f));
    CreateWGFFile(core_dir_, "a_first.wgf.json",
        CreateMinimalWGF(
            "a0000000-0000-4000-8000-000000000000", "First", 1.0f));
    CreateWGFFile(core_dir_, "m_middle.wgf.json",
        CreateMinimalWGF(
            "c0000000-0000-4000-8000-000000000000", "Middle", 1.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    auto uuids = loader_.GetUUIDList();
    ASSERT_EQ(uuids.size(), 3);

    // Should be sorted
    EXPECT_LT(uuids[0], uuids[1]);
    EXPECT_LT(uuids[1], uuids[2]);
}

TEST_F(WorldGenTest, FindByDifficulty) {
    CreateWGFFile(core_dir_, "easy.wgf.json",
        CreateMinimalWGF(
            "a0000000-0000-4000-8000-000000000001", "Easy", 1.0f));
    CreateWGFFile(core_dir_, "medium.wgf.json",
        CreateMinimalWGF(
            "a0000000-0000-4000-8000-000000000002", "Medium", 5.0f));
    CreateWGFFile(core_dir_, "hard.wgf.json",
        CreateMinimalWGF(
            "a0000000-0000-4000-8000-000000000003", "Hard", 9.0f));

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    auto easy = loader_.FindByDifficulty(0.0f, 3.0f);
    EXPECT_EQ(easy.size(), 1);
    EXPECT_EQ(easy[0]->name, "Easy");

    auto all = loader_.FindByDifficulty(0.0f, 10.0f);
    EXPECT_EQ(all.size(), 3);
}

TEST_F(WorldGenTest, FindByTags) {
    CreateWGFFile(core_dir_, "space.wgf.json", R"({
        "uuid": "a0000000-0000-4000-8000-000000000001",
        "name": "Space",
        "difficulty": 2.0,
        "tags": ["space", "easy"],
        "obstacles": []
    })");
    CreateWGFFile(core_dir_, "asteroid.wgf.json", R"({
        "uuid": "a0000000-0000-4000-8000-000000000002",
        "name": "Asteroid",
        "difficulty": 3.0,
        "tags": ["space", "asteroid"],
        "obstacles": []
    })");
    CreateWGFFile(core_dir_, "hazard.wgf.json", R"({
        "uuid": "a0000000-0000-4000-8000-000000000003",
        "name": "Hazard",
        "difficulty": 5.0,
        "tags": ["hazard"],
        "obstacles": []
    })");

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    auto space_frames = loader_.FindByTags({"space"}, false);
    EXPECT_EQ(space_frames.size(), 2);

    auto space_easy = loader_.FindByTags({"space", "easy"}, true);
    EXPECT_EQ(space_easy.size(), 1);
    EXPECT_EQ(space_easy[0]->name, "Space");
}

TEST_F(WorldGenTest, ParseCompleteWGF) {
    CreateWGFFile(core_dir_, "complete.wgf.json", R"({
        "uuid": "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",
        "name": "Complete Frame",
        "description": "A fully specified frame",
        "difficulty": 5.5,
        "tags": ["test", "complete"],
        "width": 1000,
        "spawn_rules": {
            "min_distance_from_last": 3,
            "max_frequency": 0.8,
            "requires_tags": ["space"]
        },
        "obstacles": [
            {
                "type": "destructible",
                "sprite": "images/obstacle.png",
                "position": { "x": 100, "y": 200 },
                "size": { "width": 64, "height": 64 },
                "collision": { "enabled": true, "damage": 10 },
                "health": 50
            }
        ],
        "background": {
            "layers": [
                {
                    "sprite": "images/bg.png",
                    "parallax_factor": 0.5,
                    "scroll_speed": 1.0
                }
            ]
        }
    })");

    EXPECT_TRUE(
        loader_.LoadFromDirectories(core_dir_.string(), user_dir_.string()));

    auto *wgf = loader_.GetWGFByUUID("a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d");
    ASSERT_NE(wgf, nullptr);

    EXPECT_EQ(wgf->name, "Complete Frame");
    EXPECT_EQ(wgf->description, "A fully specified frame");
    EXPECT_FLOAT_EQ(wgf->difficulty, 5.5f);
    EXPECT_EQ(wgf->tags.size(), 2);
    EXPECT_EQ(wgf->width, 1000);

    EXPECT_EQ(wgf->spawn_rules.min_distance_from_last, 3);
    EXPECT_FLOAT_EQ(wgf->spawn_rules.max_frequency, 0.8f);
    EXPECT_EQ(wgf->spawn_rules.requires_tags.size(), 1);

    ASSERT_EQ(wgf->obstacles.size(), 1);
    EXPECT_EQ(wgf->obstacles[0].type, ObstacleType::kDestructible);
    EXPECT_FLOAT_EQ(wgf->obstacles[0].position.x, 100.0f);
    EXPECT_EQ(wgf->obstacles[0].health, 50);

    ASSERT_EQ(wgf->background.layers.size(), 1);
    EXPECT_FLOAT_EQ(wgf->background.layers[0].parallax_factor, 0.5f);
}

// ============================================================================
// Global Config Tests
// ============================================================================

TEST_F(WorldGenTest, LoadGlobalConfig) {
    std::ofstream config_file(test_dir_ / "config.json");
    config_file << R"({
        "frame_width_default": 1200,
        "difficulty_scaling": {
            "base": 2.0,
            "per_frame": 0.1,
            "max": 8.0
        },
        "endless_mode": {
            "difficulty_increase_rate": 0.2,
            "max_difficulty": 9.0
        }
    })";
    config_file.close();

    EXPECT_TRUE(
        loader_.LoadGlobalConfig((test_dir_ / "config.json").string()));

    const auto &config = loader_.GetConfig();
    EXPECT_EQ(config.frame_width_default, 1200);
    EXPECT_FLOAT_EQ(config.difficulty_scaling.base, 2.0f);
    EXPECT_FLOAT_EQ(config.difficulty_scaling.per_frame, 0.1f);
    EXPECT_FLOAT_EQ(config.difficulty_scaling.max, 8.0f);
    EXPECT_FLOAT_EQ(config.endless_mode.difficulty_increase_rate, 0.2f);
    EXPECT_FLOAT_EQ(config.endless_mode.max_difficulty, 9.0f);
}

TEST_F(WorldGenTest, LoadGlobalConfigMissingFileUsesDefaults) {
    EXPECT_TRUE(loader_.LoadGlobalConfig("/nonexistent/config.json"));

    const auto &config = loader_.GetConfig();
    EXPECT_EQ(config.frame_width_default, 800);  // Default value
}

// ============================================================================
// DeterministicRNG Tests
// ============================================================================

TEST(DeterministicRNGTest, SameSeedProducesSameSequence) {
    DeterministicRNG rng1(12345);
    DeterministicRNG rng2(12345);

    // Generate 100 values and verify they're identical
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(rng1.Next(), rng2.Next()) << "Mismatch at iteration " << i;
    }
}

TEST(DeterministicRNGTest, DifferentSeedsProduceDifferentSequences) {
    DeterministicRNG rng1(12345);
    DeterministicRNG rng2(54321);

    // Generate values - they should differ
    bool any_different = false;
    for (int i = 0; i < 10; ++i) {
        if (rng1.Next() != rng2.Next()) {
            any_different = true;
            break;
        }
    }
    EXPECT_TRUE(any_different);
}

TEST(DeterministicRNGTest, ResetProducesSameSequence) {
    DeterministicRNG rng(12345);

    std::vector<uint32_t> first_run;
    for (int i = 0; i < 50; ++i) {
        first_run.push_back(rng.Next());
    }

    // Reset with same seed
    rng.SetSeed(12345);

    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(rng.Next(), first_run[i])
            << "Mismatch after reset at iteration " << i;
    }
}

TEST(DeterministicRNGTest, NextIntInRange) {
    DeterministicRNG rng(42);

    for (int i = 0; i < 1000; ++i) {
        int val = rng.NextInt(10, 20);
        EXPECT_GE(val, 10);
        EXPECT_LE(val, 20);
    }
}

TEST(DeterministicRNGTest, NextFloatInRange) {
    DeterministicRNG rng(42);

    for (int i = 0; i < 1000; ++i) {
        float val = rng.NextFloat();
        EXPECT_GE(val, 0.0f);
        EXPECT_LT(val, 1.0f);
    }
}

TEST(DeterministicRNGTest, NextBoolProbability) {
    DeterministicRNG rng(42);

    int true_count = 0;
    int total = 10000;

    for (int i = 0; i < total; ++i) {
        if (rng.NextBool(0.7f)) {
            true_count++;
        }
    }

    // Should be approximately 70% true (with some margin)
    float ratio = static_cast<float>(true_count) / total;
    EXPECT_GT(ratio, 0.65f);
    EXPECT_LT(ratio, 0.75f);
}

TEST(DeterministicRNGTest, SelectWeightedDistribution) {
    DeterministicRNG rng(42);

    std::vector<float> weights = {1.0f, 2.0f, 3.0f};  // 1/6, 2/6, 3/6
    std::vector<int> counts(3, 0);

    int total = 60000;
    for (int i = 0; i < total; ++i) {
        size_t idx = rng.SelectWeighted(weights);
        ASSERT_LT(idx, 3u);
        counts[idx]++;
    }

    // Check approximate ratios (with margin for randomness)
    float ratio0 = static_cast<float>(counts[0]) / total;
    float ratio1 = static_cast<float>(counts[1]) / total;
    float ratio2 = static_cast<float>(counts[2]) / total;

    EXPECT_GT(ratio0, 0.12f);
    EXPECT_LT(ratio0, 0.22f);  // ~16.7%
    EXPECT_GT(ratio1, 0.28f);
    EXPECT_LT(ratio1, 0.38f);  // ~33.3%
    EXPECT_GT(ratio2, 0.45f);
    EXPECT_LT(ratio2, 0.55f);  // ~50%
}

TEST(DeterministicRNGTest, ShuffleIsDeterministic) {
    DeterministicRNG rng1(12345);
    DeterministicRNG rng2(12345);

    std::vector<int> vec1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> vec2 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    rng1.Shuffle(vec1);
    rng2.Shuffle(vec2);

    EXPECT_EQ(vec1, vec2);
}

TEST(DeterministicRNGTest, StateRestoration) {
    DeterministicRNG rng(12345);

    // Generate some values
    for (int i = 0; i < 50; ++i) {
        rng.Next();
    }

    // Save state
    uint64_t saved_state = rng.GetState();
    uint64_t saved_inc = rng.GetIncrement();

    // Generate more values
    std::vector<uint32_t> values;
    for (int i = 0; i < 20; ++i) {
        values.push_back(rng.Next());
    }

    // Restore state
    rng.RestoreState(saved_state, saved_inc);

    // Should get same values
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(rng.Next(), values[i]);
    }
}

// ============================================================================
// WorldGenManager Tests
// ============================================================================

class WorldGenManagerTest : public WorldGenTest {
 protected:
    void SetUp() override {
        WorldGenTest::SetUp();

        // Create some test WGFs
        CreateWGFFile(core_dir_, "easy.wgf.json", R"({
            "uuid": "a0000000-0000-4000-8000-000000000001",
            "name": "Easy Frame",
            "difficulty": 2.0,
            "tags": ["space", "easy"],
            "width": 800,
            "obstacles": [
                {
                    "type": "static",
                    "sprite": "asteroid.png",
                    "position": {"x": 100, "y": 200},
                    "size": {"width": 64, "height": 64},
                    "collision": {"enabled": true, "damage": 10}
                }
            ]
        })");

        CreateWGFFile(core_dir_, "medium.wgf.json", R"({
            "uuid": "b0000000-0000-4000-8000-000000000002",
            "name": "Medium Frame",
            "difficulty": 5.0,
            "tags": ["space", "medium"],
            "width": 1000,
            "obstacles": [
                {
                    "type": "destructible",
                    "sprite": "asteroid.png",
                    "position": {"x": 200, "y": 300},
                    "size": {"width": 48, "height": 48},
                    "collision": {"enabled": true, "damage": 15},
                    "health": 30
                },
                {
                    "type": "hazard",
                    "sprite": "laser.png",
                    "position": {"x": 500, "y": 150},
                    "size": {"width": 32, "height": 200},
                    "collision": {"enabled": true, "damage": 25}
                }
            ]
        })");

        CreateWGFFile(core_dir_, "hard.wgf.json", R"({
            "uuid": "c0000000-0000-4000-8000-000000000003",
            "name": "Hard Frame",
            "difficulty": 8.0,
            "tags": ["space", "hard"],
            "width": 1200,
            "spawn_rules": {
                "min_distance_from_last": 2,
                "max_frequency": 0.5
            },
            "obstacles": []
        })");

        // Load the WGFs
        ASSERT_TRUE(loader_.LoadFromDirectories(
            core_dir_.string(), user_dir_.string()));
    }
};

TEST_F(WorldGenManagerTest, InitializeEndlessMode) {
    WorldGenManager manager(loader_);

    EXPECT_TRUE(manager.InitializeEndless(12345, 3.0f));
    EXPECT_TRUE(manager.IsActive());
    EXPECT_TRUE(manager.IsEndlessMode());
    EXPECT_FALSE(manager.IsLevelComplete());

    const auto &metadata = manager.GetSeedMetadata();
    EXPECT_EQ(metadata.seed_value, 12345);
    EXPECT_FLOAT_EQ(metadata.target_difficulty, 3.0f);
    EXPECT_TRUE(metadata.is_endless);
    EXPECT_EQ(metadata.allowed_wgf_uuids.size(), 3);
}

TEST_F(WorldGenManagerTest, SameSeedSameSequence) {
    WorldGenManager manager1(loader_);
    WorldGenManager manager2(loader_);

    ASSERT_TRUE(manager1.InitializeEndless(42, 5.0f));
    ASSERT_TRUE(manager2.InitializeEndless(42, 5.0f));

    // Collect frame sequences
    std::vector<std::string> sequence1;
    std::vector<std::string> sequence2;

    for (int i = 0; i < 20; ++i) {
        const auto *wgf1 = manager1.GetCurrentWGF();
        const auto *wgf2 = manager2.GetCurrentWGF();
        ASSERT_NE(wgf1, nullptr) << "wgf1 null at iteration " << i;
        ASSERT_NE(wgf2, nullptr) << "wgf2 null at iteration " << i;
        sequence1.push_back(wgf1->uuid);
        sequence2.push_back(wgf2->uuid);
        manager1.AdvanceFrame();
        manager2.AdvanceFrame();
    }

    EXPECT_EQ(sequence1, sequence2);
}

TEST_F(WorldGenManagerTest, DifferentSeedsDifferentSequences) {
    WorldGenManager manager1(loader_);
    WorldGenManager manager2(loader_);

    ASSERT_TRUE(manager1.InitializeEndless(12345, 5.0f));
    ASSERT_TRUE(manager2.InitializeEndless(54321, 5.0f));

    // Collect frame sequences
    std::vector<std::string> sequence1;
    std::vector<std::string> sequence2;

    for (int i = 0; i < 20; ++i) {
        const auto *wgf1 = manager1.GetCurrentWGF();
        const auto *wgf2 = manager2.GetCurrentWGF();
        ASSERT_NE(wgf1, nullptr) << "wgf1 null at iteration " << i;
        ASSERT_NE(wgf2, nullptr) << "wgf2 null at iteration " << i;
        sequence1.push_back(wgf1->uuid);
        sequence2.push_back(wgf2->uuid);
        manager1.AdvanceFrame();
        manager2.AdvanceFrame();
    }

    // Should differ at some point
    EXPECT_NE(sequence1, sequence2);
}

TEST_F(WorldGenManagerTest, SpawnEventsGenerated) {
    WorldGenManager manager(loader_);
    manager.InitializeEndless(12345, 3.0f);

    // Should have pending events from first frame
    EXPECT_TRUE(manager.HasPendingEvents());

    // Check we get a frame start event
    auto event = manager.PopNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->type, SpawnEvent::EventType::kFrameStart);
}

TEST_F(WorldGenManagerTest, SpawnCallbackInvoked) {
    WorldGenManager manager(loader_);

    std::vector<SpawnEvent> received_events;
    manager.SetSpawnCallback([&received_events](const SpawnEvent &event) {
        received_events.push_back(event);
    });

    manager.InitializeEndless(12345, 3.0f);

    // Should have received events
    EXPECT_FALSE(received_events.empty());

    // First event should be frame start
    EXPECT_EQ(received_events[0].type, SpawnEvent::EventType::kFrameStart);

    // Last event should be frame end
    EXPECT_EQ(received_events.back().type, SpawnEvent::EventType::kFrameEnd);
}

TEST_F(WorldGenManagerTest, ResetReproducesSequence) {
    WorldGenManager manager(loader_);
    ASSERT_TRUE(manager.InitializeEndless(12345, 5.0f));

    // Collect first sequence
    std::vector<std::string> first_sequence;
    for (int i = 0; i < 10; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        ASSERT_NE(wgf, nullptr) << "GetCurrentWGF returned nullptr at first "
                                   "sequence iteration "
                                << i;
        first_sequence.push_back(wgf->uuid);
        manager.AdvanceFrame();
    }

    // Reset
    manager.Reset();

    // Collect second sequence
    std::vector<std::string> second_sequence;
    for (int i = 0; i < 10; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        ASSERT_NE(wgf, nullptr) << "GetCurrentWGF returned nullptr at second "
                                   "sequence iteration "
                                << i;
        second_sequence.push_back(wgf->uuid);
        manager.AdvanceFrame();
    }

    EXPECT_EQ(first_sequence, second_sequence);
}

TEST_F(WorldGenManagerTest, FixedLevelMode) {
    WorldGenManager manager(loader_);

    // Create a level
    LevelDefinition level;
    level.uuid = "test-level-0000-0000-000000000001";
    level.name = "Test Level";
    level.frames = {
        "a0000000-0000-4000-8000-000000000001",  // Easy
        "b0000000-0000-4000-8000-000000000002",  // Medium
        "a0000000-0000-4000-8000-000000000001"   // Easy again
    };
    level.is_endless = false;

    manager.AddLevel(level);

    ASSERT_TRUE(manager.InitializeLevel(level.uuid));
    EXPECT_TRUE(manager.IsActive());
    EXPECT_FALSE(manager.IsEndlessMode());

    // Should play frames in order
    const auto *wgf0 = manager.GetCurrentWGF();
    ASSERT_NE(wgf0, nullptr);
    EXPECT_EQ(wgf0->uuid, level.frames[0]);
    manager.AdvanceFrame();
    const auto *wgf1 = manager.GetCurrentWGF();
    ASSERT_NE(wgf1, nullptr);
    EXPECT_EQ(wgf1->uuid, level.frames[1]);
    manager.AdvanceFrame();
    const auto *wgf2 = manager.GetCurrentWGF();
    ASSERT_NE(wgf2, nullptr);
    EXPECT_EQ(wgf2->uuid, level.frames[2]);

    // Next advance should end the level
    manager.AdvanceFrame();
    EXPECT_TRUE(manager.IsLevelComplete());
}

TEST_F(WorldGenManagerTest, LevelLoadFromString) {
    WorldGenManager manager(loader_);

    std::string level_json = R"({
        "uuid": "json-level-0000-0000-000000000001",
        "name": "JSON Level",
        "author": "Test Author",
        "frames": [
            "a0000000-0000-4000-8000-000000000001",
            "b0000000-0000-4000-8000-000000000002"
        ],
        "target_difficulty": 4.5,
        "is_endless": false
    })";

    EXPECT_TRUE(manager.LoadLevelFromString(level_json));

    const auto *level =
        manager.GetLevelByUUID("json-level-0000-0000-000000000001");
    ASSERT_NE(level, nullptr);
    EXPECT_EQ(level->name, "JSON Level");
    EXPECT_EQ(level->author, "Test Author");
    EXPECT_EQ(level->frames.size(), 2);
    EXPECT_FLOAT_EQ(level->target_difficulty, 4.5f);
}

TEST_F(WorldGenManagerTest, SaveAndRestoreState) {
    WorldGenManager manager(loader_);
    ASSERT_TRUE(manager.InitializeEndless(12345, 5.0f));

    // Advance some frames
    for (int i = 0; i < 5; ++i) {
        manager.AdvanceFrame();
    }

    // Save state
    WorldGenState saved = manager.SaveState();

    // Advance more
    std::vector<std::string> after_save;
    for (int i = 0; i < 5; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        ASSERT_NE(wgf, nullptr) << "after_save wgf null at iteration " << i;
        after_save.push_back(wgf->uuid);
        manager.AdvanceFrame();
    }

    // Restore
    ASSERT_TRUE(manager.RestoreState(saved));

    // Should get same sequence
    for (int i = 0; i < 5; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        ASSERT_NE(wgf, nullptr) << "restored wgf null at iteration " << i;
        EXPECT_EQ(wgf->uuid, after_save[i]);
        manager.AdvanceFrame();
    }
}

TEST_F(WorldGenManagerTest, StressDeterminism) {
    // Run 1000 frames and verify determinism
    WorldGenManager manager1(loader_);
    WorldGenManager manager2(loader_);

    ASSERT_TRUE(manager1.InitializeEndless(999999, 5.0f));
    ASSERT_TRUE(manager2.InitializeEndless(999999, 5.0f));

    for (int i = 0; i < 1000; ++i) {
        const auto *wgf1 = manager1.GetCurrentWGF();
        const auto *wgf2 = manager2.GetCurrentWGF();
        ASSERT_NE(wgf1, nullptr) << "manager1 wgf null at frame " << i;
        ASSERT_NE(wgf2, nullptr) << "manager2 wgf null at frame " << i;
        ASSERT_EQ(wgf1->uuid, wgf2->uuid) << "Divergence at frame " << i;
        manager1.AdvanceFrame();
        manager2.AdvanceFrame();
    }
}

}  // namespace testing
}  // namespace worldgen
