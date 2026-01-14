/**
 * @file WorldGenConfigLoader.cpp
 * @brief Implementation of the WorldGen configuration loader.
 */

#include "server/worldgen/WorldGenConfigLoader.hpp"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace worldgen {

using json = nlohmann::json;

void WorldGenConfigLoader::SetLogCallback(LogCallback callback) {
    log_callback_ = std::move(callback);
}

void WorldGenConfigLoader::Log(LogLevel level, const std::string &message) {
    if (log_callback_) {
        log_callback_(level, message);
    }
}

bool WorldGenConfigLoader::LoadFromDirectories(
    const std::string &core_path, const std::string &user_path) {
    statistics_ = LoadStatistics{};
    wgf_definitions_.clear();
    uuid_to_index_.clear();

    // Load core files first
    auto core_files = ScanDirectory(core_path);
    for (const auto &file : core_files) {
        statistics_.total_files_scanned++;
        auto wgf = LoadWGFFile(file, true);
        if (wgf.has_value()) {
            if (uuid_to_index_.contains(wgf->uuid)) {
                Log(LogLevel::kError,
                    "Duplicate UUID in core files: " + wgf->uuid +
                        " (file: " + file.string() + ")");
                statistics_.duplicate_uuids++;
                statistics_.files_skipped++;
                continue;
            }
            uuid_to_index_[wgf->uuid] = wgf_definitions_.size();
            wgf_definitions_.push_back(std::move(*wgf));
            statistics_.core_files_loaded++;
            Log(LogLevel::kInfo,
                "Loaded core WGF: " + wgf_definitions_.back().name);
        } else {
            statistics_.files_skipped++;
        }
    }

    // Load user files second
    auto user_files = ScanDirectory(user_path);
    for (const auto &file : user_files) {
        statistics_.total_files_scanned++;
        auto wgf = LoadWGFFile(file, false);
        if (wgf.has_value()) {
            if (uuid_to_index_.contains(wgf->uuid)) {
                Log(LogLevel::kWarning,
                    "User WGF has duplicate UUID (skipping): " + wgf->uuid +
                        " (file: " + file.string() + ")");
                statistics_.duplicate_uuids++;
                statistics_.files_skipped++;
                continue;
            }
            uuid_to_index_[wgf->uuid] = wgf_definitions_.size();
            wgf_definitions_.push_back(std::move(*wgf));
            statistics_.user_files_loaded++;
            Log(LogLevel::kInfo,
                "Loaded user WGF: " + wgf_definitions_.back().name);
        } else {
            statistics_.files_skipped++;
        }
    }

    // Sort UUIDs for deterministic ordering
    std::sort(wgf_definitions_.begin(), wgf_definitions_.end(),
        [](const WGFDefinition &a, const WGFDefinition &b) {
            return a.uuid < b.uuid;
        });

    // Rebuild index after sorting
    uuid_to_index_.clear();
    for (size_t i = 0; i < wgf_definitions_.size(); ++i) {
        uuid_to_index_[wgf_definitions_[i].uuid] = i;
    }

    Log(LogLevel::kInfo,
        "WorldGen loaded: " + std::to_string(statistics_.core_files_loaded) +
            " core, " + std::to_string(statistics_.user_files_loaded) +
            " user, " + std::to_string(statistics_.files_skipped) +
            " skipped");

    if (wgf_definitions_.empty()) {
        Log(LogLevel::kError,
            "No valid WGF files found! World generation will not work.");
        return false;
    }

    return true;
}

bool WorldGenConfigLoader::LoadGlobalConfig(const std::string &config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        Log(LogLevel::kWarning,
            "Config file not found, using defaults: " + config_path);
        config_ = WorldGenConfig{};
        return true;  // Not fatal, use defaults
    }

    try {
        json j = json::parse(file);

        config_.frame_width_default = j.value("frame_width_default", 800);

        if (j.contains("difficulty_scaling")) {
            const auto &ds = j["difficulty_scaling"];
            config_.difficulty_scaling.base = ds.value("base", 1.0f);
            config_.difficulty_scaling.per_frame =
                ds.value("per_frame", 0.05f);
            config_.difficulty_scaling.max = ds.value("max", 10.0f);
        }

        if (j.contains("endless_mode")) {
            const auto &em = j["endless_mode"];
            config_.endless_mode.difficulty_increase_rate =
                em.value("difficulty_increase_rate", 0.1f);
            config_.endless_mode.max_difficulty =
                em.value("max_difficulty", 10.0f);
        }

        Log(LogLevel::kInfo, "Loaded global config from: " + config_path);
        return true;
    } catch (const json::exception &e) {
        Log(LogLevel::kError,
            "Failed to parse config file: " + std::string(e.what()));
        config_ = WorldGenConfig{};
        return false;
    }
}

std::vector<std::filesystem::path> WorldGenConfigLoader::ScanDirectory(
    const std::string &path) {
    std::vector<std::filesystem::path> files;

    if (!std::filesystem::exists(path)) {
        Log(LogLevel::kWarning, "Directory does not exist: " + path);
        return files;
    }

    if (!std::filesystem::is_directory(path)) {
        Log(LogLevel::kWarning, "Path is not a directory: " + path);
        return files;
    }

    try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                const auto &filepath = entry.path();
                // Match *.wgf.json files
                if (filepath.string().ends_with(".wgf.json")) {
                    files.push_back(filepath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        Log(LogLevel::kError,
            "Filesystem error scanning " + path + ": " + e.what());
    }

    // Sort for deterministic loading order
    std::sort(files.begin(), files.end());
    return files;
}

std::optional<WGFDefinition> WorldGenConfigLoader::LoadWGFFile(
    const std::filesystem::path &filepath, bool is_core) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Log(LogLevel::kError, "Cannot open file: " + filepath.string());
        statistics_.parse_errors++;
        return std::nullopt;
    }

    json j;
    try {
        j = json::parse(file);
    } catch (const json::exception &e) {
        Log(LogLevel::kError, "JSON parse error in " + filepath.string() +
                                  ": " + std::string(e.what()));
        statistics_.parse_errors++;
        return std::nullopt;
    }

    WGFDefinition wgf;
    wgf.source_file = filepath.string();
    wgf.is_core = is_core;

    // Required fields
    if (!j.contains("uuid") || !j["uuid"].is_string()) {
        Log(LogLevel::kError,
            "Missing or invalid 'uuid' in: " + filepath.string());
        statistics_.validation_errors++;
        return std::nullopt;
    }
    wgf.uuid = j["uuid"].get<std::string>();

    if (!ValidateUUID(wgf.uuid)) {
        Log(LogLevel::kError, "Invalid UUID format in: " + filepath.string() +
                                  " (expected UUIDv4)");
        statistics_.validation_errors++;
        return std::nullopt;
    }

    if (!j.contains("name") || !j["name"].is_string()) {
        Log(LogLevel::kError,
            "Missing or invalid 'name' in: " + filepath.string());
        statistics_.validation_errors++;
        return std::nullopt;
    }
    wgf.name = j["name"].get<std::string>();

    if (!j.contains("difficulty") || !j["difficulty"].is_number()) {
        Log(LogLevel::kError,
            "Missing or invalid 'difficulty' in: " + filepath.string());
        statistics_.validation_errors++;
        return std::nullopt;
    }
    wgf.difficulty = j["difficulty"].get<float>();

    if (!j.contains("obstacles") || !j["obstacles"].is_array()) {
        Log(LogLevel::kError,
            "Missing or invalid 'obstacles' array in: " + filepath.string());
        statistics_.validation_errors++;
        return std::nullopt;
    }

    // Optional fields with defaults
    wgf.description = j.value("description", "");
    wgf.width = j.value("width", config_.frame_width_default);

    // Parse tags
    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto &tag : j["tags"]) {
            if (tag.is_string()) {
                wgf.tags.push_back(tag.get<std::string>());
            }
        }
    }

    // Parse spawn rules
    if (j.contains("spawn_rules") && j["spawn_rules"].is_object()) {
        const auto &sr = j["spawn_rules"];
        wgf.spawn_rules.min_distance_from_last =
            sr.value("min_distance_from_last", 0);
        wgf.spawn_rules.max_frequency = sr.value("max_frequency", 1.0f);

        if (sr.contains("requires_tags") && sr["requires_tags"].is_array()) {
            for (const auto &tag : sr["requires_tags"]) {
                if (tag.is_string()) {
                    wgf.spawn_rules.requires_tags.push_back(
                        tag.get<std::string>());
                }
            }
        }
    }

    // Parse obstacles
    for (const auto &obs_json : j["obstacles"]) {
        ObstacleData obstacle;

        if (!obs_json.contains("position") ||
            !obs_json["position"].is_object()) {
            Log(LogLevel::kWarning,
                "Obstacle missing position in: " + filepath.string() +
                    " (skipping obstacle)");
            continue;
        }

        const auto &pos = obs_json["position"];
        obstacle.position.x = pos.value("x", 0.0f);
        obstacle.position.y = pos.value("y", 0.0f);

        obstacle.type = StringToObstacleType(obs_json.value("type", "static"));
        obstacle.sprite = obs_json.value("sprite", "");

        if (obs_json.contains("size") && obs_json["size"].is_object()) {
            const auto &size = obs_json["size"];
            obstacle.size.width = size.value("width", 32.0f);
            obstacle.size.height = size.value("height", 32.0f);
        }

        if (obs_json.contains("collision") &&
            obs_json["collision"].is_object()) {
            const auto &coll = obs_json["collision"];
            obstacle.collision.enabled = coll.value("enabled", true);
            obstacle.collision.damage = coll.value("damage", 0);
        }

        obstacle.health = obs_json.value("health", 0);

        wgf.obstacles.push_back(std::move(obstacle));
    }

    // Parse enemies (optional)
    if (j.contains("enemies") && j["enemies"].is_array()) {
        for (const auto &enemy_json : j["enemies"]) {
            EnemySpawnData enemy;

            enemy.tag = enemy_json.value("tag", "mermaid");

            if (enemy_json.contains("position") &&
                enemy_json["position"].is_object()) {
                const auto &pos = enemy_json["position"];
                enemy.position.x = pos.value("x", 0.0f);
                enemy.position.y = pos.value("y", 0.0f);
            }

            enemy.spawn_delay = enemy_json.value("spawn_delay", 0.0f);

            wgf.enemies.push_back(std::move(enemy));
        }
    }

    // Parse background
    if (j.contains("background") && j["background"].is_object()) {
        const auto &bg = j["background"];
        if (bg.contains("layers") && bg["layers"].is_array()) {
            for (const auto &layer_json : bg["layers"]) {
                BackgroundLayer layer;
                layer.sprite = layer_json.value("sprite", "");
                layer.parallax_factor =
                    layer_json.value("parallax_factor", 1.0f);
                layer.scroll_speed = layer_json.value("scroll_speed", 1.0f);
                wgf.background.layers.push_back(std::move(layer));
            }
        }
    }

    AssignDefaults(wgf);

    if (!ValidateWGF(wgf)) {
        statistics_.validation_errors++;
        return std::nullopt;
    }

    return wgf;
}

bool WorldGenConfigLoader::ValidateWGF(const WGFDefinition &wgf) {
    // Validate difficulty range
    if (wgf.difficulty < 0.0f || wgf.difficulty > 10.0f) {
        Log(LogLevel::kWarning,
            "WGF '" + wgf.name +
                "' has difficulty outside [0, 10] range, clamping");
    }

    // Validate width
    if (wgf.width <= 0) {
        Log(LogLevel::kError, "WGF '" + wgf.name + "' has invalid width: " +
                                  std::to_string(wgf.width));
        return false;
    }

    // Validate spawn rules
    if (wgf.spawn_rules.max_frequency < 0.0f ||
        wgf.spawn_rules.max_frequency > 1.0f) {
        Log(LogLevel::kWarning,
            "WGF '" + wgf.name +
                "' has max_frequency outside [0, 1] range, clamping");
    }

    return true;
}

void WorldGenConfigLoader::AssignDefaults(WGFDefinition &wgf) {
    // Clamp difficulty
    wgf.difficulty = std::clamp(wgf.difficulty, 0.0f, 10.0f);

    // Clamp max_frequency
    wgf.spawn_rules.max_frequency =
        std::clamp(wgf.spawn_rules.max_frequency, 0.0f, 1.0f);

    // Ensure non-negative min_distance
    wgf.spawn_rules.min_distance_from_last =
        std::max(0, wgf.spawn_rules.min_distance_from_last);

    // Default width if not set
    if (wgf.width <= 0) {
        wgf.width = config_.frame_width_default;
    }
}

bool WorldGenConfigLoader::ValidateUUID(const std::string &uuid) {
    // UUIDv4 format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // where x is any hex digit and y is 8, 9, a, or b
    static const std::regex uuid_regex(
        "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{"
        "12}$",
        std::regex::icase);
    return std::regex_match(uuid, uuid_regex);
}

const WGFDefinition *WorldGenConfigLoader::GetWGFByUUID(
    const std::string &uuid) const {
    auto it = uuid_to_index_.find(uuid);
    if (it != uuid_to_index_.end()) {
        return &wgf_definitions_[it->second];
    }
    return nullptr;
}

const std::vector<WGFDefinition> &WorldGenConfigLoader::GetAllWGFs() const {
    return wgf_definitions_;
}

const WorldGenConfig &WorldGenConfigLoader::GetConfig() const {
    return config_;
}

std::vector<std::string> WorldGenConfigLoader::GetUUIDList() const {
    std::vector<std::string> uuids;
    uuids.reserve(wgf_definitions_.size());
    for (const auto &wgf : wgf_definitions_) {
        uuids.push_back(wgf.uuid);
    }
    return uuids;
}

const LoadStatistics &WorldGenConfigLoader::GetStatistics() const {
    return statistics_;
}

bool WorldGenConfigLoader::HasWGFs() const {
    return !wgf_definitions_.empty();
}

std::vector<const WGFDefinition *> WorldGenConfigLoader::FindByTags(
    const std::vector<std::string> &tags, bool match_all) const {
    std::vector<const WGFDefinition *> results;

    for (const auto &wgf : wgf_definitions_) {
        if (match_all) {
            // All tags must be present
            bool all_found = true;
            for (const auto &tag : tags) {
                if (std::find(wgf.tags.begin(), wgf.tags.end(), tag) ==
                    wgf.tags.end()) {
                    all_found = false;
                    break;
                }
            }
            if (all_found) {
                results.push_back(&wgf);
            }
        } else {
            // Any tag match
            for (const auto &tag : tags) {
                if (std::find(wgf.tags.begin(), wgf.tags.end(), tag) !=
                    wgf.tags.end()) {
                    results.push_back(&wgf);
                    break;
                }
            }
        }
    }

    return results;
}

std::vector<const WGFDefinition *> WorldGenConfigLoader::FindByDifficulty(
    float min_difficulty, float max_difficulty) const {
    std::vector<const WGFDefinition *> results;

    for (const auto &wgf : wgf_definitions_) {
        if (wgf.difficulty >= min_difficulty &&
            wgf.difficulty <= max_difficulty) {
            results.push_back(&wgf);
        }
    }

    return results;
}

}  // namespace worldgen
