# WorldGen System - Complete Documentation

**Complete World Generation System for R-Type J.A.M.E.S.**

## Overview

The WorldGen system provides deterministic, seed-based world generation using **WorldGen Frames (WGF)** - reusable world segments with obstacles, backgrounds, and metadata. The system supports:

- **Deterministic Generation**: Same seed always produces identical worlds
- **User Modding**: Drop new WGF files into a folder to add content
- **Level Editor Support**: Levels are simple JSON lists of WGF UUIDs
- **Difficulty Scaling**: Dynamic difficulty adjustment in endless mode
- **Save/Load**: Complete state serialization for game saves

## Architecture

```
server/
├── include/server/worldgen/
│   ├── WorldGen.hpp              # Convenience header (includes all)
│   ├── WorldGenTypes.hpp         # POD data structures
│   ├── WorldGenConfigLoader.hpp  # WGF file loading/validation
│   ├── WorldGenManager.hpp       # Runtime generation manager
│   └── DeterministicRNG.hpp      # PCG-based PRNG
├── src/server/worldgen/
│   ├── WorldGenConfigLoader.cpp  # JSON parsing/validation
│   └── WorldGenManager.cpp       # Frame selection/spawn events
├── include/server/systems/
│   └── WorldGenSystem.hpp        # ECS integration
├── src/server/Systems/
│   └── WorldGenSystem.cpp        # Entity spawning
└── assets/worldgen/
    ├── config.json               # Global settings
    ├── core/*.wgf.json          # Built-in frames
    ├── user/*.wgf.json          # User mods (optional)
    └── levels/*.level.json      # Level definitions
```

## Quick Start

### Basic Usage

```cpp
#include "server/worldgen/WorldGen.hpp"
#include "server/systems/WorldGenSystem.hpp"

// Initialize system
server::WorldGenSystem worldgen;
worldgen.Initialize("assets/worldgen");

// Start endless mode with seed
worldgen.StartEndless(12345, 2.0f);  // seed, initial_difficulty

// Or start a predefined level
worldgen.StartLevel("Tutorial - First Flight");

// In game loop
worldgen.Update(delta_time, scroll_speed, registry);
```

### Using WorldGenManager Directly

```cpp
#include "server/worldgen/WorldGen.hpp"

worldgen::WorldGenConfigLoader loader;
loader.LoadFromDirectories("assets/worldgen/core", "assets/worldgen/user");
loader.LoadGlobalConfig("assets/worldgen/config.json");

worldgen::WorldGenManager manager(loader);

// Set callback for spawn events
manager.SetSpawnCallback([](const worldgen::SpawnEvent& event) {
    if (event.type == worldgen::SpawnEvent::EventType::kObstacle) {
        // Create entity from event data
    }
});

// Start endless mode
manager.InitializeEndless(12345, 3.0f);

// In game loop
manager.Update(delta_time, scroll_speed);
while (manager.HasPendingEvents()) {
    auto event = manager.PopNextEvent();
    // Process event...
}
```

## WGF File Format

**Required:** `uuid`, `name`, `difficulty`, `obstacles`  
**Optional:** `description`, `width`, `tags`, `spawn_rules`, `background`

```json
{
  "uuid": "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",
  "name": "Asteroid Field",
  "difficulty": 4.5,
  "tags": ["space", "obstacles"],
  "width": 800,
  "spawn_rules": {
    "min_distance_from_last": 3,
    "max_frequency": 0.8,
    "requires_tags": ["space"]
  },
  "obstacles": [
    {
      "type": "destructible",
      "sprite": "asteroid.png",
      "position": {"x": 200, "y": 150},
      "size": {"width": 48, "height": 48},
      "collision": {"enabled": true, "damage": 10},
      "health": 20
    }
  ],
  "background": {
    "layers": [
      {
        "sprite": "nebula.png",
        "parallax_factor": 0.3,
        "scroll_speed": 0.5
      }
    ]
  }
}
```

**Obstacle Types:** `static`, `destructible`, `hazard`, `decoration`

## Level File Format

Levels are simple JSON files listing WGF UUIDs in order:

```json
{
  "uuid": "level-tutorial-001-4000-8000-000000000001",
  "name": "Tutorial - First Flight",
  "author": "R-Type Team",
  "description": "Learn the basics",
  "target_difficulty": 1.5,
  "is_endless": false,
  "frames": [
    "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",
    "b2c3d4e5-f6a7-4b8c-9d0e-1f2a3b4c5d6e",
    "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d"
  ]
}
```

If `is_endless` is true, procedural generation continues after the listed frames.

## Key Types

### SpawnEvent

```cpp
struct SpawnEvent {
    enum class EventType {
        kObstacle,   // Spawn an obstacle
        kFrameStart, // New frame begins
        kFrameEnd,   // Frame ends
        kLevelEnd    // Level complete (fixed mode)
    };
    EventType type;
    std::string wgf_uuid;
    size_t obstacle_index;
    float world_x, world_y;
    int frame_number;
    // Cached obstacle data
    ObstacleType obstacle_type;
    std::string sprite;
    Size2f size;
    CollisionData collision;
    int health;
};
```

### SeedMetadata

```cpp
struct SeedMetadata {
    uint64_t seed_value;
    float target_difficulty;
    bool is_endless;
    std::vector<std::string> allowed_wgf_uuids;  // Ensures determinism
    std::string level_uuid;  // For fixed levels
    uint64_t creation_timestamp;
};
```

### DeterministicRNG

PCG-based PRNG for reproducible generation:

```cpp
worldgen::DeterministicRNG rng(12345);
uint32_t value = rng.Next();
float f = rng.NextFloat();           // [0, 1)
int i = rng.NextInt(0, 100);         // [0, 100]
size_t idx = rng.SelectWeighted(weights);  // Weighted selection
```

## Determinism Guarantees

1. **Same seed = same world**: Verified by unit tests up to 1000+ frames
2. **UUID stability**: Adding new WGFs doesn't change existing seeds (UUIDs are embedded in seed metadata)
3. **Platform independence**: PCG algorithm produces identical results across all platforms
4. **Frame-rate independence**: Generation based on world offset, not frame count

## Map Editor Integration

Creating a level editor is straightforward:

1. **Load WGFs**: Use `WorldGenConfigLoader::GetAllWGFs()` to get available frames
2. **Build sequence**: User drags/drops frames to create a sequence
3. **Save level**: Output a JSON file with the frame UUIDs
4. **Preview**: Use `WorldGenManager::AdvanceFrame()` to preview frames

```cpp
// Example: Save level from editor
void SaveLevel(const std::string& name, const std::vector<std::string>& frames) {
    json level;
    level["uuid"] = GenerateUUID();  // Generate new UUID
    level["name"] = name;
    level["frames"] = frames;
    level["is_endless"] = false;
    // Write to file...
}

// Example: List available WGFs for editor palette
void PopulateEditorPalette(worldgen::WorldGenConfigLoader& loader) {
    for (const auto& wgf : loader.GetAllWGFs()) {
        AddToPalette(wgf.uuid, wgf.name, wgf.difficulty, wgf.tags);
    }
}
```

## Configuration

### Global Config (config.json)

```json
{
  "frame_width_default": 800,
  "difficulty_scaling": {
    "base": 1.0,
    "per_frame": 0.05,
    "max": 10.0
  },
  "endless_mode": {
    "difficulty_increase_rate": 0.1,
    "max_difficulty": 10.0
  }
}
```

## Testing

```bash
cmake --build build
ctest --test-dir build --output-on-failure -R "WorldGen"
```

Tests cover:
- Config loading and validation
- UUID format validation
- Duplicate handling
- Deterministic RNG (statistical tests)
- Seed reproducibility (stress test: 1000 frames)
- Level loading
- Save/restore state

## User Modding

1. Create a `.wgf.json` file following the schema
2. Generate a unique UUID: `uuidgen` or online generator
3. Place in `assets/worldgen/user/`
4. File is loaded automatically on next server start

**Rules:**
- User WGFs with duplicate core UUIDs are skipped
- Invalid files are skipped with warning
- User mods don't affect existing seeds (UUID list is embedded in seed metadata)

## Error Handling

- Malformed JSON → skip file, log error
- Missing required fields → skip file
- Invalid UUID format → skip file
- Duplicate UUIDs → user file skipped, core file kept
- Missing WGF in level → warning, skip frame

## API Reference

### WorldGenConfigLoader

```cpp
bool LoadFromDirectories(const std::string& core, const std::string& user);
bool LoadGlobalConfig(const std::string& path);
const WGFDefinition* GetWGFByUUID(const std::string& uuid) const;
const std::vector<WGFDefinition>& GetAllWGFs() const;
std::vector<std::string> GetUUIDList() const;
std::vector<const WGFDefinition*> FindByTags(const std::vector<std::string>& tags, bool match_all) const;
std::vector<const WGFDefinition*> FindByDifficulty(float min, float max) const;
```

### WorldGenManager

```cpp
bool InitializeEndless(uint64_t seed, float initial_difficulty);
uint64_t InitializeEndlessRandom(float initial_difficulty);
bool InitializeLevel(const std::string& level_uuid);
void Update(float delta_time, float scroll_speed);
std::optional<SpawnEvent> PopNextEvent();
bool HasPendingEvents() const;
void SetSpawnCallback(SpawnEventCallback callback);
bool AdvanceFrame();
const WorldGenState& GetState() const;
WorldGenState SaveState() const;
bool RestoreState(const WorldGenState& state);
```

### WorldGenSystem (ECS Integration)

```cpp
bool Initialize(const std::string& base_path);
bool StartEndless(uint64_t seed, float initial_difficulty);
bool StartLevel(const std::string& level_name);
void Update(float delta_time, float scroll_speed, Engine::registry& registry);
void Stop();
void Reset();
std::vector<std::string> GetAvailableLevels() const;
```
