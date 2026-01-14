# Enemy Generation System

**Design Document** - Data-driven enemy spawning integrated with WorldGen frames

## Overview

The enemy generation system extends WGF (WorldGen Frame) files to include enemy spawns. Enemy configuration is stored in WGF files, and `FactoryActors` handles entity creation using asset JSON files.

## Architecture

```
server/
├── assets/worldgen/
│   ├── core/
│   │   ├── enemy_wave_alpha.wgf.json      ← enemy-focused frame
│   │   └── mermaid_patrol.wgf.json        ← enemy patrol frame
│   └── levels/*.level.json
└── src/server/Systems/WorldGenSystem.cpp   ← spawns enemies from events

client/
├── assets/data/
│   ├── mermaid.json                       ← enemy stats
│   ├── kamifish.json                      ← enemy stats
│   └── player.json
└── game/factory/factory_ennemies/
    └── FactoryActors.hpp                  ← enemy factory
```

## WGF Schema - Enemy Array

Each WGF file can include an `enemies` array with spawn definitions:

```json
{
  "uuid": "a1b2c3d4-...",
  "name": "Enemy Wave Alpha",
  "difficulty": 3.0,
  "obstacles": [],
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
  ]
}
```

### Enemy Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `tag` | string | ✅ | Enemy type identifier (matches `FactoryActors` and `assets/data/*.json`) |
| `position` | `{x, y}` | ✅ | Spawn position relative to frame start (world-x added at runtime) |
| `spawn_delay` | float | ❌ | Delay in seconds after frame starts (default: 0.0) |

### Available Enemy Tags

| Tag | JSON Config | Description |
|-----|-------------|-------------|
| `mermaid` | `mermaid.json` | Standard enemy with sine wave movement |
| `kamifish` | `kamifish.json` | Kamikaze enemy that targets players |

New enemy types can be added by:
1. Creating `assets/data/<enemy>.json` with stats
2. Adding handler in `FactoryActors::Create()`
3. Using the tag in WGF files

## Data Structures

### EnemySpawnData (`WorldGenTypes.hpp`)

```cpp
/**
 * @brief Enemy spawn definition within a WGF.
 */
struct EnemySpawnData {
    std::string tag;             ///< Enemy type tag (e.g., "mermaid", "kamifish")
    Vec2f position;              ///< Spawn position relative to frame start
    float spawn_delay = 0.0f;    ///< Delay after frame starts (seconds)
};

// In WGFDefinition:
struct WGFDefinition {
    // ... existing fields ...
    std::vector<EnemySpawnData> enemies;  ///< Enemy spawn definitions
};
```

### SpawnEvent with Enemy Support

```cpp
struct SpawnEvent {
    enum class EventType : uint8_t {
        kObstacle = 0,   // Spawn an obstacle
        kEnemy = 1,      // Spawn an enemy
        kFrameStart = 2, // Frame begins
        kFrameEnd = 3,   // Frame ends
        kLevelEnd = 4    // Level complete
    };

    EventType type;
    std::string wgf_uuid;
    size_t obstacle_index;       // For kObstacle
    float world_x, world_y;      // Absolute position
    int frame_number;

    // For kObstacle events:
    ObstacleType obstacle_type;
    std::string sprite;
    Size2f size;
    CollisionData collision;
    int health;

    // For kEnemy events:
    std::string enemy_tag;       // e.g., "mermaid"
};
```

## Implementation

### Loading (WorldGenConfigLoader)

```cpp
// In ParseWGF(), after parsing obstacles:
if (json_data.contains("enemies") && json_data["enemies"].is_array()) {
    for (const auto& e : json_data["enemies"]) {
        EnemySpawnData spawn;
        spawn.tag = e.value("tag", "mermaid");
        spawn.spawn_delay = e.value("spawn_delay", 0.0f);
        
        if (e.contains("position")) {
            spawn.position.x = e["position"].value("x", 0.0f);
            spawn.position.y = e["position"].value("y", 0.0f);
        }
        
        wgf.enemies.push_back(spawn);
    }
}
```

### Runtime Spawning (WorldGenManager)

When a frame starts, `WorldGenManager` queues `SpawnEvent` objects for each enemy:

```cpp
void WorldGenManager::GenerateEnemyEvents(const WGFDefinition& wgf, 
                                           float frame_x_offset, 
                                           int frame_num) {
    for (size_t i = 0; i < wgf.enemies.size(); i++) {
        const auto& enemy = wgf.enemies[i];
        
        SpawnEvent event;
        event.type = SpawnEvent::EventType::kEnemy;
        event.wgf_uuid = wgf.uuid;
        event.frame_number = frame_num;
        event.world_x = frame_x_offset + enemy.position.x;
        event.world_y = enemy.position.y;
        event.enemy_tag = enemy.tag;
        
        // Note: spawn_delay is handled by caller or tracked separately
        pending_events_.push(std::move(event));
    }
}
```

### Entity Creation (WorldGenSystem)

```cpp
void WorldGenSystem::ProcessSpawnEvent(const worldgen::SpawnEvent& event,
                                        Engine::registry& registry) {
    if (event.type == worldgen::SpawnEvent::EventType::kEnemy) {
        // Use FactoryActors to create enemy
        FactoryActors::GetInstance().Create(
            event.enemy_tag, registry, event.world_x, event.world_y);
    }
    // ... handle other event types ...
}
```

## Example WGF Files

### Enemy Wave (enemies only)

```json
{
  "uuid": "wgf-enemy-wave-alpha-0001-0000-000000000001",
  "name": "Enemy Wave Alpha",
  "difficulty": 3.0,
  "tags": ["enemies", "wave"],
  "obstacles": [],
  "enemies": [
    {
      "tag": "mermaid",
      "position": {"x": 50, "y": 150},
      "spawn_delay": 0.0
    },
    {
      "tag": "mermaid",
      "position": {"x": 50, "y": 350},
      "spawn_delay": 0.0
    },
    {
      "tag": "kamifish",
      "position": {"x": 200, "y": 250},
      "spawn_delay": 1.0
    }
  ]
}
```

### Mixed Frame (obstacles + enemies)

```json
{
  "uuid": "wgf-asteroid-ambush-0001-0000-000000000001",
  "name": "Asteroid Ambush",
  "difficulty": 5.0,
  "tags": ["space", "obstacles", "enemies"],
  "obstacles": [
    {
      "type": "destructible",
      "sprite": "asteroid.png",
      "position": {"x": 100, "y": 200},
      "size": {"width": 48, "height": 48},
      "collision": {"enabled": true, "damage": 10},
      "health": 20
    }
  ],
  "enemies": [
    {
      "tag": "mermaid",
      "position": {"x": 300, "y": 100},
      "spawn_delay": 0.5
    },
    {
      "tag": "mermaid",
      "position": {"x": 300, "y": 500},
      "spawn_delay": 0.5
    }
  ]
}
```

## Integration

### Uses Existing Systems

| Component | Purpose |
|-----------|---------|
| `FactoryActors` | Creates enemy entities from tag |
| `PatternMovement` | Enemy AI movement (configured in JSON) |
| `mermaid.json`, etc. | Enemy stats and configuration |
| `WorldGenManager` | Queues spawn events |
| `WorldGenSystem` | Processes events and creates entities |

### Data Flow

```
1. WGF JSON loaded → EnemySpawnData stored in WGFDefinition
2. Frame selected by WorldGenManager
3. Enemy events queued with world coordinates
4. WorldGenSystem processes events
5. FactoryActors::Create() builds entity
6. PatternMovement system handles AI (from JSON config)
```

## Testing

```cpp
TEST_F(WorldGenTest, ParseEnemySpawns) {
    auto wgf = LoadWGFWithEnemies("test_enemy_wave.wgf.json");
    ASSERT_EQ(wgf.enemies.size(), 2);
    
    EXPECT_EQ(wgf.enemies[0].tag, "mermaid");
    EXPECT_FLOAT_EQ(wgf.enemies[0].spawn_delay, 0.0f);
    EXPECT_FLOAT_EQ(wgf.enemies[0].position.x, 100.0f);
    
    EXPECT_EQ(wgf.enemies[1].tag, "kamifish");
    EXPECT_FLOAT_EQ(wgf.enemies[1].spawn_delay, 1.5f);
}
```

## Advantages

1. **Simple schema**: Just `tag`, `position`, `spawn_delay`
2. **Uses existing factory**: Enemy configuration stays in `assets/data/*.json`
3. **Deterministic**: Same seed + frame = same enemy spawns
4. **Extensible**: Add new enemy types via `FactoryActors`
5. **Moddable**: Users create custom WGFs in `user/` folder
