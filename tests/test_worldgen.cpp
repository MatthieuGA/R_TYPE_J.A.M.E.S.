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

}  // namespace testing
}  // namespace worldgen
