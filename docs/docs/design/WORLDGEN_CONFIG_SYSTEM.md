# WorldGen Configuration System

**Issue #125** - JSON-based config system for world generation frames

## Overview

The WorldGen config system loads JSON-defined **WorldGen Frames (WGF)** - reusable world segments with obstacles, enemies, backgrounds, and metadata. Designers can create/modify levels without code changes.

**Key Features:** UUID-based determinism, enemy spawning, user modding support, validated parsing, ECS-decoupled design

## Architecture

```
server/
├── include/server/worldgen/
│   ├── WorldGen.hpp              # Convenience header
│   ├── WorldGenTypes.hpp         # POD data structures
│   ├── WorldGenConfigLoader.hpp  # Loader interface
│   ├── WorldGenManager.hpp       # Runtime manager
│   └── DeterministicRNG.hpp      # PCG-based PRNG
├── src/server/worldgen/
│   ├── WorldGenConfigLoader.cpp  # JSON parsing/validation
│   └── WorldGenManager.cpp       # Frame selection/spawn events
└── assets/worldgen/
    ├── config.json               # Global settings
    ├── core/*.wgf.json          # 10 built-in frames
    ├── user/*.wgf.json          # User mods (gitignored)
    └── levels/*.level.json      # Level definitions
```

## WGF File Format

**Required:** `uuid`, `name`, `difficulty`, `obstacles`  
**Optional:** `description`, `width`, `tags`, `spawn_rules`, `enemies`, `background`

```json
{
  "uuid": "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",
  "name": "Asteroid Field",
  "difficulty": 4.5,
  "tags": ["space", "obstacles", "enemies"],
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
  "enemies": [
    {
      "tag": "mermaid",
      "position": {"x": 100, "y": 200},
      "spawn_delay": 0.0
    },
    {
      "tag": "kamifish",
      "position": {"x": 300, "y": 400},
      "spawn_delay": 1.5
    }
  ],
  "spawn_rules": {
    "min_distance_from_last": 3,
    "max_frequency": 0.8
  }
}
```

**Obstacle Types:** `static`, `destructible`, `hazard`, `decoration`

**Enemy Fields:**
- `tag`: Enemy type from FactoryActors (e.g., `"mermaid"`, `"kamifish"`)
- `position`: Spawn position relative to frame
- `spawn_delay`: Delay after frame starts (default: 0)

## C++ API

### Usage Example

```cpp
#include "server/worldgen/WorldGen.hpp"

worldgen::WorldGenConfigLoader loader;
loader.LoadFromDirectories("assets/worldgen/core", "assets/worldgen/user");

worldgen::WorldGenManager manager(loader);
manager.InitializeEndless(12345, 2.0f);  // seed, difficulty

// In game loop
manager.Update(delta_time, scroll_speed);
while (auto event = manager.PopNextEvent()) {
    if (event->type == SpawnEvent::EventType::kObstacle) { /* ... */ }
    else if (event->type == SpawnEvent::EventType::kEnemy) { /* ... */ }
}
```

### Key Types

```cpp
struct WGFDefinition {
    std::string uuid, name, description;
    float difficulty;
    std::vector<std::string> tags;
    int width;
    SpawnRules spawn_rules;
    std::vector<ObstacleData> obstacles;
    std::vector<EnemySpawnData> enemies;
    BackgroundData background;
};

struct EnemySpawnData {
    std::string tag;             // Enemy type (e.g., "mermaid")
    Vec2f position;              // Spawn position
    float spawn_delay = 0.0f;    // Delay after frame starts
};

struct SpawnEvent {
    enum class EventType { kObstacle, kEnemy, kFrameStart, kFrameEnd, kLevelEnd };
    EventType type;
    float world_x, world_y;
    std::string enemy_tag;       // For kEnemy events
    // ... obstacle fields for kObstacle events
};
```

### Key Methods

- `LoadFromDirectories()` - Load WGFs from core/user dirs
- `GetWGFByUUID()` - O(1) lookup by UUID
- `FindByDifficulty()` / `FindByTags()` - Query WGFs
- `InitializeEndless()` - Start endless mode with seed
- `Update()` / `PopNextEvent()` - Runtime generation

## Validation & Error Handling

**UUID Rules:**
- Must be valid UUIDv4: `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`
- User WGFs with duplicate core UUIDs are skipped

**File Processing:**
- Malformed JSON → skip file, log error
- Missing required fields → skip file
- Invalid values → skip file

## User Modding

**Create custom WGF:** Place `.wgf.json` in `server/assets/worldgen/user/`

```json
{
  "uuid": "12345678-1234-4123-8123-123456789abc",
  "name": "My Frame",
  "difficulty": 3.0,
  "obstacles": [...],
  "enemies": [{"tag": "mermaid", "position": {"x": 100, "y": 200}}]
}
```

**Rules:** User files load after core. Duplicate UUIDs are skipped. Generate UUIDs with `uuidgen` or online tools. User files are gitignored.

## Testing

**20+ unit tests** in `tests/test_worldgen.cpp` covering loading, validation, querying, RNG, and runtime generation.

```bash
cmake --build build
ctest --test-dir build -R WorldGen --output-on-failure
```

## See Also

- `WORLDGEN_COMPLETE.md` - Full system documentation
- `WORLDGEN_IMPLEMENTATION.md` - Implementation summary
- `ENEMY_GENERATION.md` - Enemy spawning design
