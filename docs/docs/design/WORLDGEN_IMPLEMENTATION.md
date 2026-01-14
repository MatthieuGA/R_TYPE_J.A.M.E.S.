# WorldGen Configuration System - Implementation Summary

**Issue #125** - Data-driven configuration system for world generation

This document summarizes how issue #125 was implemented, mapping each requirement to the solution.

---

## Objective Fulfillment

| Requirement | Status | Implementation |
|-------------|--------|----------------|
| Deterministic for any seed | ✅ | UUID list sorted alphabetically, PRNG seeded once |
| Extendable (drop new WGF files) | ✅ | `core/` and `user/` directory scanning |
| Stable (new files don't change seeds) | ✅ | Seed embeds UUID list at generation time |
| Configurable via JSON | ✅ | All WGF and config files are JSON |
| Compatible with level editors | ✅ | `LevelDefinition` struct for ordered UUID lists |

---

## Architecture Overview

```
server/
├── include/server/worldgen/
│   ├── WorldGen.hpp              ← Convenience header
│   ├── WorldGenTypes.hpp         ← POD data structures
│   └── WorldGenConfigLoader.hpp  ← Loader interface
├── src/server/worldgen/
│   └── WorldGenConfigLoader.cpp  ← JSON parsing & validation
└── assets/worldgen/
    ├── config.json               ← Global configuration
    ├── core/*.wgf.json          ← Built-in frames (6 examples)
    └── user/*.wgf.json          ← User mods (optional)
```

**Design Choice**: External Manager Pattern (not ECS-integrated)
- `WorldGenConfigLoader` loads once at server startup
- Typed C++ structs exposed to game systems
- ECS never stores JSON - only typed `WGFDefinition` data

---

## Schema Implementations

### WGF Schema (`*.wgf.json`)

```json
{
  "uuid": "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",  // Required: UUIDv4
  "name": "Asteroid Field",                         // Required: 1-64 chars
  "difficulty": 3.5,                                // Required: 0.0-10.0
  "description": "Dense asteroid cluster",          // Optional
  "tags": ["space", "obstacles"],                   // Optional: category tags
  "width": 800,                                     // Optional: frame width
  "spawn_rules": {                                  // Optional
    "min_distance_from_last": 3,
    "max_frequency": 0.8,
    "requires_tags": ["space"]
  },
  "obstacles": [                                    // Required (can be empty)
    {
      "type": "destructible",
      "sprite": "asteroid.png",
      "position": {"x": 200, "y": 150},
      "size": {"width": 48, "height": 48},
      "collision": {"enabled": true, "damage": 10},
      "health": 20
    }
  ],
  "background": {                                   // Optional
    "layers": [
      {"sprite": "stars.png", "parallax_factor": 0.3, "scroll_speed": 0.5}
    ]
  }
}
```

### Global Config (`config.json`)

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

### Level Definition (for Editor)

```json
{
  "name": "Level 1",
  "author": "Designer",
  "frames": [
    "uuid-empty-space",
    "uuid-asteroid-light",
    "uuid-asteroid-dense"
  ]
}
```

---

## Data Structures (C++)

### Core Types (`WorldGenTypes.hpp`)

| Struct | Purpose |
|--------|---------|
| `Vec2f`, `Size2f` | 2D position/size |
| `ObstacleType` | Enum: kStatic, kDestructible, kHazard, kDecoration |
| `ObstacleData` | Single obstacle definition |
| `BackgroundLayer` | Parallax layer config |
| `SpawnRules` | Frame selection constraints |
| `WGFDefinition` | Complete frame definition |
| `DifficultyScaling` | Difficulty progression rules |
| `EndlessModeConfig` | Endless mode parameters |
| `WorldGenConfig` | Global configuration |
| `LevelDefinition` | Editor-created level (UUID list) |

### Loader (`WorldGenConfigLoader`)

| Method | Purpose |
|--------|---------|
| `LoadFromDirectories(core, user)` | Scan and load all WGF files |
| `LoadGlobalConfig(path)` | Load `config.json` |
| `GetWGFByUUID(uuid)` | O(1) lookup by UUID |
| `GetUUIDList()` | Sorted list for determinism |
| `FindByDifficulty(min, max)` | Query by difficulty range |
| `FindByTags(tags)` | Query by category tags |
| `GetStatistics()` | Load success/failure counts |

---

## Checklist Resolution

### Config File Structure ✅

- [x] **WGF schema defined**: `WGFDefinition` struct with all fields documented
- [x] **Seed metadata schema**: `SeedMetadata` struct designed (runtime PR)
- [x] **Root config file**: `config.json` with difficulty scaling and endless mode

### Library & Parsing ✅

- [x] **Chosen**: `nlohmann-json` via vcpkg (industry standard, header-only)

### Modules / Architecture ✅

- [x] **WorldGenConfigLoader**: Scans core + user directories
- [x] **Validation**: UUID format, required fields, value ranges
- [x] **Default values**: Applied for optional fields
- [x] **UUID mapping**: Hash map for O(1) lookup
- [x] **Deterministic list**: Sorted alphabetically after load

### Deterministic WorldGen Logic ⏳ (Next PR)

- [ ] **WGF selection algorithm**: Uses seed, difficulty, UUID list
- [ ] **Endless mode**: Difficulty scaling per frame index

### Integration into ECS ✅

- [x] **Strategy chosen**: External Manager Pattern
- [x] **Documented**: Config loader provides typed data, ECS spawns entities
- [x] **No JSON in ECS**: Only `WGFDefinition` structs passed to systems

### Error Handling ✅

- [x] **Empty folders**: Returns false, logs warning
- [x] **Corrupt files**: Skipped with parse error logged
- [x] **Missing fields**: Skipped with validation error logged
- [x] **Duplicate UUIDs**: User file skipped, core takes precedence
- [x] **Logging**: Configurable callback with Info/Warning/Error levels

### User Mod Support ✅

- [x] **Two directories**: `core/` (shipped) + `user/` (mods)
- [x] **Core precedence**: User can't override core UUIDs
- [x] **Seed stability**: UUID list embedded at seed creation time

### Documentation ✅

- [x] **Schema docs**: `docs/WORLDGEN_CONFIG_SYSTEM.md`
- [x] **User mod guide**: `server/assets/worldgen/user/README.md`
- [x] **Seed behavior**: Explained in design docs
- [x] **Determinism rules**: Sorted UUID list, embedded at seed time

---

## Acceptance Criteria Status

| Criterion | Status |
|-----------|--------|
| Config system loads at server startup | ✅ Ready |
| WGF files parsed with correct metadata | ✅ Implemented |
| Seeds embed UUID list, difficulty, endless flag | ⏳ Runtime PR |
| New WGFs don't impact old seeds | ✅ Design ensures this |
| World generation uses config | ⏳ Runtime PR |
| Error handling prevents crashes | ✅ Graceful skip + logging |
| Logs report success/failure | ✅ Statistics + callback |
| Architecture documented | ✅ External Manager Pattern |
| Supports level editor | ✅ `LevelDefinition` struct |

---

## Files Delivered in This PR

### Source Code

| File | Lines | Purpose |
|------|-------|---------|
| `WorldGenTypes.hpp` | 180 | All POD data structures |
| `WorldGenConfigLoader.hpp` | 173 | Loader interface |
| `WorldGenConfigLoader.cpp` | 476 | JSON parsing & validation |
| `WorldGen.hpp` | 20 | Convenience include |

### Assets

| File | Purpose |
|------|---------|
| `config.json` | Global configuration |
| `empty_space.wgf.json` | Example: no obstacles |
| `asteroid_field_light.wgf.json` | Example: few asteroids |
| `asteroid_field_dense.wgf.json` | Example: many asteroids |
| `debris_corridor.wgf.json` | Example: corridor layout |
| `hazard_zone.wgf.json` | Example: hazards |
| `gauntlet_run.wgf.json` | Example: mixed challenge |
| `user/README.md` | Modding guide |

### Tests

| File | Tests | Coverage |
|------|-------|----------|
| `test_worldgen.cpp` | 13 | Loading, validation, queries |

### Documentation

| File | Purpose |
|------|---------|
| `WORLDGEN_CONFIG_SYSTEM.md` | Complete system documentation |

---

## What's Next (Future PRs)

### Runtime Generation PR
- `WorldGenManager` class for seed-based frame selection
- `SeedMetadata` struct with embedded UUID list
- PRNG integration for deterministic selection
- Difficulty weighting algorithm
- Endless mode difficulty scaling
- Server tick integration

### Enemy Generation PR
- Extend WGF schema with `enemies` array
- `EnemySpawn` struct with pattern/formation
- Integration with existing `PatternMovement` system
- See `docs/docs/design/ENEMY_GENERATION.md`

### Lua Scripting PR (Optional)
- `LuaScript` component for boss AI
- Multi-phase behavior support
- See `docs/docs/design/LUA_ENEMY_SCRIPTS.md`

---

## Usage Example

```cpp
#include "server/worldgen/WorldGen.hpp"

// At server startup
worldgen::WorldGenConfigLoader loader;
loader.SetLogCallback([](worldgen::LogLevel level, const std::string& msg) {
    std::cout << "[WorldGen] " << msg << std::endl;
});

if (!loader.LoadFromDirectories("assets/worldgen/core", "assets/worldgen/user")) {
    std::cerr << "Failed to load WGF files!" << std::endl;
}
loader.LoadGlobalConfig("assets/worldgen/config.json");

// Query loaded frames
auto easy_frames = loader.FindByDifficulty(0.0f, 3.0f);
auto space_frames = loader.FindByTags({"space"});

// Get specific frame
const auto* frame = loader.GetWGFByUUID("a1b2c3d4-...");
if (frame) {
    for (const auto& obstacle : frame->obstacles) {
        // Spawn obstacle entities...
    }
}

// Statistics
const auto& stats = loader.GetStatistics();
std::cout << "Loaded: " << stats.core_files_loaded << " core, "
          << stats.user_files_loaded << " user" << std::endl;
```

---

## References

- **nlohmann/json**: https://github.com/nlohmann/json
- **UUID RFC 4122**: https://www.rfc-editor.org/rfc/rfc4122
- **Inspiration**: RimWorld mod system, Factorio data packs, Minecraft datapacks
