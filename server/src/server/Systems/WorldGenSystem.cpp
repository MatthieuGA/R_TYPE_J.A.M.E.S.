/**
 * @file WorldGenSystem.cpp
 * @brief Implementation of the WorldGen ECS integration system.
 */

#include "server/systems/WorldGenSystem.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

namespace server {

WorldGenSystem::WorldGenSystem()
    : config_loader_(nullptr), manager_(nullptr), initialized_(false) {}

WorldGenSystem::WorldGenSystem(
    worldgen::WorldGenManager &manager, Engine::registry &registry)
    : config_loader_(nullptr),
      manager_(&manager),
      initialized_(true),
      registry_(&registry) {
    // Set up spawn event callback to create entities
    manager_->SetSpawnCallback([this](const worldgen::SpawnEvent &event) {
        if (registry_) {
            ProcessSpawnEvent(event, *registry_);
        }
    });
}

bool WorldGenSystem::Initialize(const std::string &base_path) {
    config_loader_ = std::make_unique<worldgen::WorldGenConfigLoader>();

    // Set up logging
    config_loader_->SetLogCallback(
        [](worldgen::LogLevel level, const std::string &msg) {
            std::string prefix;
            switch (level) {
                case worldgen::LogLevel::kInfo:
                    prefix = "[WorldGen INFO] ";
                    break;
                case worldgen::LogLevel::kWarning:
                    prefix = "[WorldGen WARN] ";
                    break;
                case worldgen::LogLevel::kError:
                    prefix = "[WorldGen ERROR] ";
                    break;
                case worldgen::LogLevel::kFatal:
                    prefix = "[WorldGen FATAL] ";
                    break;
            }
            std::cout << prefix << msg << std::endl;
        });

    // Construct paths
    std::string core_path = base_path + "/core";
    std::string user_path = base_path + "/user";
    std::string config_path = base_path + "/config.json";
    std::string levels_path = base_path + "/levels";

    // Load global config
    config_loader_->LoadGlobalConfig(config_path);

    // Load WGF files
    if (!config_loader_->LoadFromDirectories(core_path, user_path)) {
        std::cerr << "[WorldGenSystem] Failed to load WGF files" << std::endl;
        return false;
    }

    // Create manager
    manager_owned_ =
        std::make_unique<worldgen::WorldGenManager>(*config_loader_);
    manager_ = manager_owned_.get();
    manager_->SetLogCallback(
        [](worldgen::LogLevel level, const std::string &msg) {
            std::string prefix;
            switch (level) {
                case worldgen::LogLevel::kInfo:
                    prefix = "[WorldGenManager INFO] ";
                    break;
                case worldgen::LogLevel::kWarning:
                    prefix = "[WorldGenManager WARN] ";
                    break;
                case worldgen::LogLevel::kError:
                    prefix = "[WorldGenManager ERROR] ";
                    break;
                case worldgen::LogLevel::kFatal:
                    prefix = "[WorldGenManager FATAL] ";
                    break;
            }
            std::cout << prefix << msg << std::endl;
        });

    // Load levels if directory exists
    if (std::filesystem::exists(levels_path)) {
        LoadLevels(levels_path);
    }

    initialized_ = true;
    std::cout << "[WorldGenSystem] Initialized successfully with "
              << config_loader_->GetAllWGFs().size() << " WGFs and "
              << manager_->GetAllLevels().size() << " levels" << std::endl;

    return true;
}

bool WorldGenSystem::StartEndless(uint64_t seed, float initial_difficulty) {
    if (!initialized_ || !manager_) {
        std::cerr << "[WorldGenSystem] Not initialized" << std::endl;
        return false;
    }
    return manager_->InitializeEndless(seed, initial_difficulty);
}

uint64_t WorldGenSystem::StartEndlessRandom(float initial_difficulty) {
    if (!initialized_ || !manager_) {
        std::cerr << "[WorldGenSystem] Not initialized" << std::endl;
        return 0;
    }
    return manager_->InitializeEndlessRandom(initial_difficulty);
}

bool WorldGenSystem::StartLevel(const std::string &level_name) {
    if (!initialized_ || !manager_) {
        std::cerr << "[WorldGenSystem] Not initialized" << std::endl;
        return false;
    }

    // Find level by name
    for (const auto &level : manager_->GetAllLevels()) {
        if (level.name == level_name) {
            return manager_->InitializeLevel(level.uuid);
        }
    }

    std::cerr << "[WorldGenSystem] Level not found: " << level_name
              << std::endl;
    return false;
}

bool WorldGenSystem::StartLevelByUUID(const std::string &level_uuid) {
    if (!initialized_ || !manager_) {
        std::cerr << "[WorldGenSystem] Not initialized" << std::endl;
        return false;
    }
    return manager_->InitializeLevel(level_uuid);
}

void WorldGenSystem::Update(
    float delta_time, float scroll_speed, Engine::registry &registry) {
    if (!initialized_ || !manager_ || !manager_->IsActive()) {
        return;
    }

    // Update worldgen state
    // When using the callback-based constructor, events are processed
    // immediately via the callback, so we don't need to process the queue
    // here. The callback was set in the constructor for immediate processing.
    manager_->Update(delta_time, scroll_speed);

    // Only process queue if we're NOT using the callback (Initialize() path)
    // If registry_ is set, we're using the callback constructor
    if (!registry_) {
        while (manager_->HasPendingEvents()) {
            auto event = manager_->PopNextEvent();
            if (event.has_value()) {
                ProcessSpawnEvent(*event, registry);
            }
        }
    }
}

void WorldGenSystem::Stop() {
    if (manager_) {
        manager_->Stop();
    }
}

void WorldGenSystem::Reset() {
    if (manager_) {
        manager_->Reset();
    }
}

float WorldGenSystem::GetWorldOffset() const {
    return manager_ ? manager_->GetWorldOffset() : 0.0f;
}

float WorldGenSystem::GetCurrentDifficulty() const {
    return manager_ ? manager_->GetCurrentDifficulty() : 0.0f;
}

int WorldGenSystem::GetCurrentFrame() const {
    return manager_ ? manager_->GetCurrentFrameIndex() : 0;
}

bool WorldGenSystem::IsActive() const {
    return manager_ && manager_->IsActive();
}

bool WorldGenSystem::IsLevelComplete() const {
    return manager_ && manager_->IsLevelComplete();
}

bool WorldGenSystem::IsEndless() const {
    return manager_ && manager_->IsEndlessMode();
}

uint64_t WorldGenSystem::GetCurrentSeed() const {
    return manager_ ? manager_->GetSeedMetadata().seed_value : 0;
}

int WorldGenSystem::LoadLevels(const std::string &levels_path) {
    if (!manager_) {
        return 0;
    }

    int count = 0;
    try {
        for (const auto &entry :
            std::filesystem::directory_iterator(levels_path)) {
            if (entry.is_regular_file()) {
                const auto &path = entry.path();
                if (path.string().ends_with(".level.json")) {
                    if (manager_->LoadLevelFromFile(path.string())) {
                        count++;
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "[WorldGenSystem] Error scanning levels: " << e.what()
                  << std::endl;
    }

    std::cout << "[WorldGenSystem] Loaded " << count << " levels from "
              << levels_path << std::endl;
    return count;
}

std::vector<std::string> WorldGenSystem::GetAvailableLevels() const {
    std::vector<std::string> names;
    if (manager_) {
        for (const auto &level : manager_->GetAllLevels()) {
            names.push_back(level.name);
        }
    }
    return names;
}

worldgen::WorldGenManager *WorldGenSystem::GetManager() {
    return manager_;
}

worldgen::WorldGenConfigLoader *WorldGenSystem::GetConfigLoader() {
    return config_loader_.get();
}

void WorldGenSystem::ProcessSpawnEvent(
    const worldgen::SpawnEvent &event, Engine::registry &registry) {
    switch (event.type) {
        case worldgen::SpawnEvent::EventType::kObstacle:
            SpawnObstacle(event, registry);
            break;

        case worldgen::SpawnEvent::EventType::kEnemy:
            SpawnEnemy(event, registry);
            break;

        case worldgen::SpawnEvent::EventType::kFrameStart:
            // Log frame transition (compact message)
            // std::cout << "[WorldGen] Frame " << event.frame_number
            //           << " started" << std::endl;
            break;

        case worldgen::SpawnEvent::EventType::kFrameEnd:
            // Could trigger frame completion effects here
            break;

        case worldgen::SpawnEvent::EventType::kLevelEnd:
            // std::cout << "[WorldGen] Level complete!" << std::endl;
            // Could trigger victory screen or next level transition
            break;
    }
}

void WorldGenSystem::SpawnObstacle(
    const worldgen::SpawnEvent &event, Engine::registry &registry) {
    // Create entity
    auto entity = registry.spawn_entity();

    // Calculate spawn position (world_x is absolute, world_y from WGF)
    // We spawn off-screen to the right
    float spawn_x = screen_width_ + (event.world_x - GetWorldOffset());
    float spawn_y = event.world_y;

    // Add Transform component
    registry.AddComponent<Component::Transform>(
        entity, Component::Transform{spawn_x, spawn_y, 0.0f, {1.0f, 1.0f}});

    // Add Velocity for scrolling (obstacles move left with the world)
    // The scroll speed will be applied by a movement system
    registry.AddComponent<Component::Velocity>(
        entity, Component::Velocity{-200.0f, 0.0f, 0.0f, 0.0f});

    // Add HitBox for collision
    registry.AddComponent<Component::HitBox>(
        entity, Component::HitBox{
                    event.size.width, event.size.height, true, 0.0f, 0.0f});

    // Add Solid for collision response (locked = cannot be pushed)
    registry.AddComponent<Component::Solid>(
        entity, Component::Solid{true, true});

    // Add NetworkId for network sync
    registry.AddComponent<Component::NetworkId>(
        entity, Component::NetworkId{Server::GetNextNetworkId()});

    // Add ObstacleTag for entity type identification
    // Subtype: 0 = Asteroid (destructible), 1 = Wall (indestructible/static)
    uint8_t subtype =
        (event.obstacle_type == worldgen::ObstacleType::kStatic) ? 1 : 0;
    registry.AddComponent<Component::ObstacleTag>(
        entity, Component::ObstacleTag{subtype});

    // Add health for destructible obstacles
    if (event.obstacle_type == worldgen::ObstacleType::kDestructible &&
        event.health > 0) {
        registry.AddComponent<Component::Health>(
            entity, Component::Health{event.health});
    }

    // Brief log message (reduced verbosity)
    // std::cout << "[WorldGen] Obstacle spawned at (" << spawn_x << ", "
    //           << spawn_y << ") size=" << event.size.width << "x"
    //           << event.size.height << std::endl;
}

void WorldGenSystem::SpawnEnemy(
    const worldgen::SpawnEvent &event, Engine::registry &registry) {
    // Calculate spawn position (spawn off-screen to the right)
    float spawn_x = screen_width_ + (event.world_x - GetWorldOffset());
    float spawn_y = event.world_y;

    // Create entity
    auto entity = registry.spawn_entity();

    // Use FactoryActors to create the enemy with proper sprites and components
    FactoryActors::GetInstance().CreateActor(
        entity, registry, event.enemy_tag, {spawn_x, spawn_y}, false);

    // std::cout << "[WorldGen] Spawned enemy '" << event.enemy_tag << "' at ("
    //           << spawn_x << ", " << spawn_y << ")" << std::endl;
}

}  // namespace server
