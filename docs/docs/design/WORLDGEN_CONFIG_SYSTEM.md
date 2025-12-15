# WorldGen Configuration System

**Issue #125** - JSON-based config system for world generation frames

## Overview

The WorldGen config system loads JSON-defined **WorldGen Frames (WGF)** - reusable world segments with obstacles, backgrounds, and metadata. Designers can create/modify levels without code changes.

**Key Features:** UUID-based determinism, user modding support, validated parsing, ECS-decoupled design

## Architecture

```
server/
├── include/server/worldgen/
│   ├── WorldGen.hpp              # Convenience header
│   ├── WorldGenTypes.hpp         # POD data structures
│   └── WorldGenConfigLoader.hpp  # Loader interface
├── src/server/worldgen/
│   └── WorldGenConfigLoader.cpp  # JSON parsing/validation
└── assets/worldgen/
    ├── config.json               # Global settings
    ├── core/*.wgf.json          # 6 built-in frames
    └── user/*.wgf.json          # User mods (optional)
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
  "spawn_rules": {
    "min_distance_from_last": 3,
    "max_frequency": 0.8
  }
}
```

**Obstacle Types:** `static`, `destructible`, `hazard`, `decoration`

## C++ API

### Usage Example

```cpp
#include "server/worldgen/WorldGen.hpp"

worldgen::WorldGenConfigLoader loader;
loader.LoadFromDirectories("assets/worldgen/core", "assets/worldgen/user");
loader.LoadGlobalConfig("assets/worldgen/config.json");

// Access WGF by UUID
const auto* wgf = loader.GetWGFByUUID("a1b2c3d4-...");

// Query by criteria
auto easy_frames = loader.FindByDifficulty(0.0f, 3.0f);
auto space_frames = loader.FindByTags({"space"}, false);

// Check statistics
const auto& stats = loader.GetStatistics();
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
    BackgroundData background;
};

struct ObstacleData {
    ObstacleType type;  // kStatic, kDestructible, kHazard, kDecoration
    std::string sprite;
    Vec2f position;
    Size2f size;
    CollisionData collision;
    int health;
};
```

### Key Methods

- `LoadFromDirectories()` - Load WGFs from core/user dirs
- `GetWGFByUUID()` - O(1) lookup by UUID
- `FindByDifficulty()` / `FindByTags()` - Query WGFs
- `GetStatistics()` - Load stats (files loaded/skipped/errors)

## Validation & Error Handling

**UUID Rules:**
- Must be valid UUIDv4: `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`
- User WGFs with duplicate core UUIDs are skipped

**File Processing:**
- Malformed JSON → skip file, log error
- Missing required fields → skip file
- Invalid values → skip file

**Graceful Degradation:**
```cpp
if (!loader.LoadFromDirectories("core", "user")) {
    // Use fallback or minimal procedural generation
}
if (stats.files_skipped > 0) {
    // Check stats.parse_errors, validation_errors, duplicate_uuids
}
```

## User Modding

**Create custom WGF:** Place `.wgf.json` in `server/assets/worldgen/user/`

```json
{
  "uuid": "12345678-1234-4123-8123-123456789abc",
  "name": "My Frame",
  "difficulty": 3.0,
  "obstacles": [...]
}
```

**Rules:** User files load after core. Duplicate UUIDs are skipped. Generate UUIDs with `uuidgen` or online tools.

## Testing

**13 unit tests** in `tests/test_worldgen.cpp` covering loading, validation, querying, and config parsing.

```bash
cmake --build build --target worldgen_tests
./build/tests/worldgen_tests --gtest_brief=1
```

## Build Integration

**Dependencies:** `nlohmann-json` (via vcpkg)

```cmake
# server/CMakeLists.txt
target_sources(r-type_server PRIVATE
    src/server/worldgen/WorldGenConfigLoader.cpp)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(r-type_server PRIVATE nlohmann_json::nlohmann_json)
```
