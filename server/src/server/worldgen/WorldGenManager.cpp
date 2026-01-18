/**
 * @file WorldGenManager.cpp
 * @brief Implementation of the WorldGen runtime manager.
 */

#include "server/worldgen/WorldGenManager.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace worldgen {

using json = nlohmann::json;

WorldGenManager::WorldGenManager(const WorldGenConfigLoader &config_loader)
    : config_loader_(config_loader),
      rng_(),
      state_(),
      spawn_callback_(nullptr),
      log_callback_(nullptr) {}

void WorldGenManager::SetLogCallback(LogCallback callback) {
    log_callback_ = std::move(callback);
}

void WorldGenManager::Log(LogLevel level, const std::string &message) {
    if (log_callback_) {
        log_callback_(level, message);
    }
}

// ============================================================================
// Initialization
// ============================================================================

bool WorldGenManager::InitializeEndless(
    uint64_t seed, float initial_difficulty) {
    if (!config_loader_.HasWGFs()) {
        Log(LogLevel::kError, "Cannot initialize: no WGFs loaded");
        return false;
    }

    // Reset state
    state_ = WorldGenState{};
    event_queue_ = std::queue<SpawnEvent>();
    recent_frame_uuids_.clear();
    current_wgf_uuid_.clear();
    current_frame_end_x_ = 0.0f;
    next_frame_in_level_ = 0;

    // Create seed metadata capturing current WGF library state
    state_.seed_metadata.seed_value = seed;
    state_.seed_metadata.target_difficulty =
        std::clamp(initial_difficulty, 0.0f, 10.0f);
    state_.seed_metadata.is_endless = true;
    state_.seed_metadata.allowed_wgf_uuids = config_loader_.GetUUIDList();
    state_.seed_metadata.creation_timestamp = static_cast<uint64_t>(
        std::chrono::system_clock::now().time_since_epoch().count());

    // Initialize RNG
    rng_.SetSeed(seed);
    state_.rng_state = rng_.GetState();
    state_.rng_increment = rng_.GetIncrement();

    // Set initial difficulty
    state_.current_difficulty = initial_difficulty;
    state_.is_active = true;
    state_.level_complete = false;

    Log(LogLevel::kInfo,
        "Initialized endless mode with seed: " + std::to_string(seed) +
            ", difficulty: " + std::to_string(initial_difficulty) +
            ", WGFs available: " +
            std::to_string(state_.seed_metadata.allowed_wgf_uuids.size()));

    // Generate first frame immediately
    AdvanceFrame();

    return true;
}

uint64_t WorldGenManager::InitializeEndlessRandom(float initial_difficulty) {
    // Generate random seed from system clock
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t seed = static_cast<uint64_t>(now.time_since_epoch().count());

    if (InitializeEndless(seed, initial_difficulty)) {
        return seed;
    }
    return 0;
}

bool WorldGenManager::InitializeLevel(const std::string &level_uuid) {
    const LevelDefinition *level = GetLevelByUUID(level_uuid);
    if (!level) {
        Log(LogLevel::kError, "Level not found: " + level_uuid);
        return false;
    }

    if (level->frames.empty()) {
        Log(LogLevel::kError, "Level has no frames: " + level_uuid);
        return false;
    }

    // Reset state
    state_ = WorldGenState{};
    event_queue_ = std::queue<SpawnEvent>();
    recent_frame_uuids_.clear();
    current_wgf_uuid_.clear();
    current_frame_end_x_ = 0.0f;
    next_frame_in_level_ = 0;

    // Create seed metadata for fixed level
    state_.seed_metadata.seed_value = 0;  // Not used for fixed levels
    state_.seed_metadata.target_difficulty = level->target_difficulty;
    state_.seed_metadata.is_endless = level->is_endless;
    state_.seed_metadata.level_uuid = level_uuid;

    // Only include WGFs that are in the level
    for (const auto &frame_uuid : level->frames) {
        if (config_loader_.GetWGFByUUID(frame_uuid) != nullptr) {
            state_.seed_metadata.allowed_wgf_uuids.push_back(frame_uuid);
        }
    }

    state_.seed_metadata.creation_timestamp = static_cast<uint64_t>(
        std::chrono::system_clock::now().time_since_epoch().count());

    // Initialize RNG (used for variations within frames if needed)
    rng_.SetSeed(std::hash<std::string>{}(level_uuid));
    state_.rng_state = rng_.GetState();
    state_.rng_increment = rng_.GetIncrement();

    state_.current_difficulty = level->target_difficulty;
    state_.is_active = true;
    state_.level_complete = false;

    Log(LogLevel::kInfo, "Initialized level: " + level->name + " with " +
                             std::to_string(level->frames.size()) + " frames");

    // Generate first frame
    AdvanceFrame();

    return true;
}

bool WorldGenManager::InitializeFromMetadata(const SeedMetadata &metadata) {
    // Reset state
    state_ = WorldGenState{};
    event_queue_ = std::queue<SpawnEvent>();
    recent_frame_uuids_.clear();
    current_wgf_uuid_.clear();
    current_frame_end_x_ = 0.0f;
    next_frame_in_level_ = 0;

    state_.seed_metadata = metadata;

    // Validate that we have the required WGFs
    int missing_count = 0;
    for (const auto &uuid : metadata.allowed_wgf_uuids) {
        if (config_loader_.GetWGFByUUID(uuid) == nullptr) {
            Log(LogLevel::kWarning, "Missing WGF from seed metadata: " + uuid);
            missing_count++;
        }
    }

    if (missing_count > 0 && static_cast<size_t>(missing_count) ==
                                 metadata.allowed_wgf_uuids.size()) {
        Log(LogLevel::kError, "All WGFs from seed metadata are missing!");
        return false;
    }

    // Initialize RNG
    rng_.SetSeed(metadata.seed_value);
    state_.rng_state = rng_.GetState();
    state_.rng_increment = rng_.GetIncrement();

    state_.current_difficulty = metadata.target_difficulty;
    state_.is_active = true;
    state_.level_complete = false;

    Log(LogLevel::kInfo,
        "Initialized from metadata, seed: " +
            std::to_string(metadata.seed_value) +
            ", endless: " + (metadata.is_endless ? "yes" : "no"));

    AdvanceFrame();
    return true;
}

void WorldGenManager::Reset() {
    if (!state_.is_active && state_.seed_metadata.seed_value == 0 &&
        state_.seed_metadata.level_uuid.empty()) {
        Log(LogLevel::kWarning, "Cannot reset: worldgen not initialized");
        return;
    }

    // Store the metadata before clearing state
    SeedMetadata saved_metadata = state_.seed_metadata;

    // Re-initialize from the stored metadata
    InitializeFromMetadata(saved_metadata);
}

void WorldGenManager::Stop() {
    state_.is_active = false;
    Log(LogLevel::kInfo, "WorldGen stopped");
}

// ============================================================================
// Runtime Operations
// ============================================================================

void WorldGenManager::Update(float delta_time, float scroll_speed) {
    if (!state_.is_active || state_.level_complete) {
        return;
    }

    // Update world offset
    float scroll_delta = scroll_speed * delta_time;
    state_.world_offset += scroll_delta;

    // Check if we need to generate the next frame
    while (state_.world_offset >= current_frame_end_x_ - 400.0f) {
        // Generate next frame when we're within 400 units of the current
        // frame's end
        if (!AdvanceFrame()) {
            break;  // No more frames available
        }
    }

    // Update difficulty based on progression
    UpdateDifficulty();
}

std::optional<SpawnEvent> WorldGenManager::PeekNextEvent() const {
    if (event_queue_.empty()) {
        return std::nullopt;
    }
    return event_queue_.front();
}

std::optional<SpawnEvent> WorldGenManager::PopNextEvent() {
    if (event_queue_.empty()) {
        return std::nullopt;
    }
    SpawnEvent event = event_queue_.front();
    event_queue_.pop();
    return event;
}

bool WorldGenManager::HasPendingEvents() const {
    return !event_queue_.empty();
}

void WorldGenManager::SetSpawnCallback(SpawnEventCallback callback) {
    spawn_callback_ = std::move(callback);
}

void WorldGenManager::ClearCallback() {
    spawn_callback_ = nullptr;
}

bool WorldGenManager::AdvanceFrame() {
    std::string next_uuid;

    if (state_.seed_metadata.is_endless ||
        state_.seed_metadata.level_uuid.empty()) {
        // Endless mode: select next frame based on difficulty
        next_uuid = SelectNextWGF();
    } else {
        // Fixed level mode: use predetermined sequence
        const LevelDefinition *level =
            GetLevelByUUID(state_.seed_metadata.level_uuid);
        if (!level) {
            Log(LogLevel::kError, "Level no longer available");
            state_.level_complete = true;
            return false;
        }

        if (static_cast<size_t>(next_frame_in_level_) >=
            level->frames.size()) {
            if (level->is_endless) {
                // Continue with procedural generation
                next_uuid = SelectNextWGF();
            } else {
                // Level complete
                SpawnEvent end_event;
                end_event.type = SpawnEvent::EventType::kLevelEnd;
                end_event.frame_number = state_.current_frame_index;
                end_event.world_x = current_frame_end_x_;
                event_queue_.push(end_event);
                if (spawn_callback_) {
                    spawn_callback_(end_event);
                }
                state_.level_complete = true;
                Log(LogLevel::kInfo, "Level complete!");
                return false;
            }
        } else {
            next_uuid = level->frames[next_frame_in_level_];
            next_frame_in_level_++;
        }
    }

    if (next_uuid.empty()) {
        Log(LogLevel::kWarning, "No WGF available for selection");
        return false;
    }

    const WGFDefinition *wgf = config_loader_.GetWGFByUUID(next_uuid);
    if (!wgf) {
        Log(LogLevel::kError, "Selected WGF not found: " + next_uuid);
        return false;
    }

    // Generate events for this frame
    float frame_start_x = current_frame_end_x_;
    // std::cout << "[WorldGen] Generating frame: '" << wgf->name << "' with "
    //           << wgf->obstacles.size() << " obstacles and "
    //           << wgf->enemies.size() << " enemies" << std::endl;
    GenerateFrameEvents(*wgf, frame_start_x);

    // Update tracking
    current_wgf_uuid_ = next_uuid;
    current_frame_end_x_ = frame_start_x + static_cast<float>(wgf->width);
    state_.current_frame_index++;

    // Update frame history
    recent_frame_uuids_.push_back(next_uuid);
    if (recent_frame_uuids_.size() > kMaxFrameHistory) {
        recent_frame_uuids_.erase(recent_frame_uuids_.begin());
    }

    // Save RNG state
    state_.rng_state = rng_.GetState();
    state_.rng_increment = rng_.GetIncrement();

    Log(LogLevel::kInfo, "Advanced to frame " +
                             std::to_string(state_.current_frame_index) +
                             ": " + wgf->name);

    return true;
}

// ============================================================================
// State Queries
// ============================================================================

const WorldGenState &WorldGenManager::GetState() const {
    return state_;
}

const SeedMetadata &WorldGenManager::GetSeedMetadata() const {
    return state_.seed_metadata;
}

int WorldGenManager::GetCurrentFrameIndex() const {
    return state_.current_frame_index;
}

float WorldGenManager::GetWorldOffset() const {
    return state_.world_offset;
}

float WorldGenManager::GetCurrentDifficulty() const {
    return state_.current_difficulty;
}

bool WorldGenManager::IsActive() const {
    return state_.is_active;
}

bool WorldGenManager::IsLevelComplete() const {
    return state_.level_complete;
}

bool WorldGenManager::IsEndlessMode() const {
    return state_.seed_metadata.is_endless;
}

const WGFDefinition *WorldGenManager::GetCurrentWGF() const {
    if (current_wgf_uuid_.empty()) {
        return nullptr;
    }
    return config_loader_.GetWGFByUUID(current_wgf_uuid_);
}

// ============================================================================
// Level Management
// ============================================================================

bool WorldGenManager::LoadLevelFromFile(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Log(LogLevel::kError, "Cannot open level file: " + filepath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return LoadLevelFromString(buffer.str());
}

bool WorldGenManager::LoadLevelFromString(const std::string &json_content) {
    try {
        json j = json::parse(json_content);

        LevelDefinition level;

        // Required fields
        if (!j.contains("uuid") || !j["uuid"].is_string()) {
            Log(LogLevel::kError, "Level missing 'uuid' field");
            return false;
        }
        level.uuid = j["uuid"].get<std::string>();

        if (!j.contains("name") || !j["name"].is_string()) {
            Log(LogLevel::kError, "Level missing 'name' field");
            return false;
        }
        level.name = j["name"].get<std::string>();

        if (!j.contains("frames") || !j["frames"].is_array()) {
            Log(LogLevel::kError, "Level missing 'frames' array");
            return false;
        }

        for (const auto &frame : j["frames"]) {
            if (frame.is_string()) {
                level.frames.push_back(frame.get<std::string>());
            }
        }

        // Optional fields
        level.author = j.value("author", "Unknown");
        level.description = j.value("description", "");
        level.target_difficulty = j.value("target_difficulty", 5.0f);
        level.is_endless = j.value("is_endless", false);

        AddLevel(level);
        Log(LogLevel::kInfo, "Loaded level: " + level.name);
        return true;
    } catch (const json::exception &e) {
        Log(LogLevel::kError,
            "Failed to parse level JSON: " + std::string(e.what()));
        return false;
    }
}

void WorldGenManager::AddLevel(const LevelDefinition &level) {
    // Check for duplicate
    auto it = level_uuid_to_index_.find(level.uuid);
    if (it != level_uuid_to_index_.end()) {
        // Replace existing
        levels_[it->second] = level;
        Log(LogLevel::kWarning, "Replaced existing level: " + level.uuid);
    } else {
        level_uuid_to_index_[level.uuid] = levels_.size();
        levels_.push_back(level);
    }
}

const LevelDefinition *WorldGenManager::GetLevelByUUID(
    const std::string &uuid) const {
    auto it = level_uuid_to_index_.find(uuid);
    if (it != level_uuid_to_index_.end()) {
        return &levels_[it->second];
    }
    return nullptr;
}

const std::vector<LevelDefinition> &WorldGenManager::GetAllLevels() const {
    return levels_;
}

// ============================================================================
// Save/Load Support
// ============================================================================

WorldGenState WorldGenManager::SaveState() const {
    WorldGenState saved = state_;
    saved.current_wgf_uuid = current_wgf_uuid_;
    return saved;
}

bool WorldGenManager::RestoreState(const WorldGenState &saved_state) {
    state_ = saved_state;

    // Restore the current WGF uuid
    current_wgf_uuid_ = saved_state.current_wgf_uuid;

    // Restore RNG state
    rng_.RestoreState(saved_state.rng_state, saved_state.rng_increment);

    // Clear event queue (will be regenerated)
    event_queue_ = std::queue<SpawnEvent>();

    Log(LogLevel::kInfo, "Restored worldgen state at frame " +
                             std::to_string(state_.current_frame_index));
    return true;
}

// ============================================================================
// Private Methods
// ============================================================================

std::string WorldGenManager::SelectNextWGF() {
    // Build list of candidate WGFs from the allowed list
    std::vector<std::pair<std::string, float>> candidates;

    for (const auto &uuid : state_.seed_metadata.allowed_wgf_uuids) {
        const WGFDefinition *wgf = config_loader_.GetWGFByUUID(uuid);
        if (!wgf)
            continue;

        if (!CanSelectWGF(*wgf))
            continue;

        float weight = CalculateDifficultyWeight(*wgf);
        if (weight > 0.0f) {
            candidates.emplace_back(uuid, weight);
        }
    }

    if (candidates.empty()) {
        // Fallback: allow any WGF from the allowed list
        for (const auto &uuid : state_.seed_metadata.allowed_wgf_uuids) {
            const WGFDefinition *wgf = config_loader_.GetWGFByUUID(uuid);
            if (wgf) {
                candidates.emplace_back(uuid, 1.0f);
            }
        }
    }

    if (candidates.empty()) {
        return "";
    }

    // Build weight vector and select
    std::vector<float> weights;
    weights.reserve(candidates.size());
    for (const auto &c : candidates) {
        weights.push_back(c.second);
    }

    size_t selected_index = rng_.SelectWeighted(weights);
    return candidates[selected_index].first;
}

void WorldGenManager::GenerateFrameEvents(
    const WGFDefinition &wgf, float frame_start_x) {
    // Frame start event
    SpawnEvent start_event;
    start_event.type = SpawnEvent::EventType::kFrameStart;
    start_event.wgf_uuid = wgf.uuid;
    start_event.frame_number = state_.current_frame_index;
    start_event.world_x = frame_start_x;
    event_queue_.push(start_event);
    if (spawn_callback_) {
        spawn_callback_(start_event);
    }

    // Generate obstacle events
    for (size_t i = 0; i < wgf.obstacles.size(); ++i) {
        const auto &obstacle = wgf.obstacles[i];

        SpawnEvent event;
        event.type = SpawnEvent::EventType::kObstacle;
        event.wgf_uuid = wgf.uuid;
        event.obstacle_index = i;
        event.frame_number = state_.current_frame_index;

        // Calculate world position
        event.world_x = frame_start_x + obstacle.position.x;
        event.world_y = obstacle.position.y;

        // Cache obstacle data
        event.obstacle_type = obstacle.type;
        event.sprite = obstacle.sprite;
        event.size = obstacle.size;
        event.collision = obstacle.collision;
        event.health = obstacle.health;

        event_queue_.push(event);
        if (spawn_callback_) {
            spawn_callback_(event);
        }
    }

    // Generate enemy spawn events
    for (size_t i = 0; i < wgf.enemies.size(); ++i) {
        const auto &enemy = wgf.enemies[i];

        SpawnEvent event;
        event.type = SpawnEvent::EventType::kEnemy;
        event.wgf_uuid = wgf.uuid;
        event.obstacle_index = i;  // Reuse for enemy index
        event.frame_number = state_.current_frame_index;

        // Calculate world position
        event.world_x = frame_start_x + enemy.position.x;
        event.world_y = enemy.position.y;

        // Set enemy tag
        event.enemy_tag = enemy.tag;

        event_queue_.push(event);
        if (spawn_callback_) {
            spawn_callback_(event);
        }
    }

    // Frame end event
    SpawnEvent end_event;
    end_event.type = SpawnEvent::EventType::kFrameEnd;
    end_event.wgf_uuid = wgf.uuid;
    end_event.frame_number = state_.current_frame_index;
    end_event.world_x = frame_start_x + static_cast<float>(wgf.width);
    event_queue_.push(end_event);
    if (spawn_callback_) {
        spawn_callback_(end_event);
    }
}

float WorldGenManager::CalculateDifficultyWeight(
    const WGFDefinition &wgf) const {
    // Weight based on how close the WGF difficulty is to target
    float diff = std::abs(wgf.difficulty - state_.current_difficulty);

    // Gaussian-like falloff: weight decreases with distance from target
    float weight = std::exp(-diff * diff / 4.0f);

    // Apply max_frequency from spawn rules
    weight *= wgf.spawn_rules.max_frequency;

    return weight;
}

bool WorldGenManager::CanSelectWGF(const WGFDefinition &wgf) const {
    // Check min_distance_from_last
    if (wgf.spawn_rules.min_distance_from_last > 0) {
        int min_dist = wgf.spawn_rules.min_distance_from_last;
        int check_count =
            std::min(static_cast<int>(recent_frame_uuids_.size()), min_dist);

        for (int i = 0; i < check_count; ++i) {
            size_t idx = recent_frame_uuids_.size() - 1 - i;
            if (recent_frame_uuids_[idx] == wgf.uuid) {
                return false;  // Too recently used
            }
        }
    }

    // Check requires_tags (previous frame must have these tags)
    if (!wgf.spawn_rules.requires_tags.empty() &&
        !recent_frame_uuids_.empty()) {
        const std::string &last_uuid = recent_frame_uuids_.back();
        const WGFDefinition *last_wgf = config_loader_.GetWGFByUUID(last_uuid);

        if (last_wgf) {
            for (const auto &required_tag : wgf.spawn_rules.requires_tags) {
                bool found = false;
                for (const auto &tag : last_wgf->tags) {
                    if (tag == required_tag) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;  // Previous frame missing required tag
                }
            }
        }
    }

    return true;
}

void WorldGenManager::UpdateDifficulty() {
    if (!state_.seed_metadata.is_endless) {
        return;  // Fixed levels have constant difficulty
    }

    const auto &config = config_loader_.GetConfig();

    // Increase difficulty based on frames completed
    float new_difficulty = state_.seed_metadata.target_difficulty +
                           static_cast<float>(state_.current_frame_index) *
                               config.endless_mode.difficulty_increase_rate;

    state_.current_difficulty =
        std::min(new_difficulty, config.endless_mode.max_difficulty);
}

}  // namespace worldgen
