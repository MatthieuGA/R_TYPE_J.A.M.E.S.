# Enemy Generation System

**Design Document** - Data-driven enemy spawning integrated with WorldGen frames

## Overview

The enemy generation system extends WGF (WorldGen Frame) files to include timed enemy spawns. Enemies use the existing `PatternMovement` component system for AI behaviors.

## Architecture

```
server/assets/worldgen/
├── core/
│   ├── asteroid_field_light.wgf.json  ← obstacles + enemies
│   └── enemy_wave_alpha.wgf.json      ← enemy-focused frame
└── user/

client/
├── assets/data/
│   ├── mermaid.json                   ← enemy stats (existing)
│   └── grunt.json                     ← new enemy type
├── include/components/
│   └── GameplayComponents.hpp         ← PatternMovement (existing)
└── game/factory/factory_ennemies/
    └── FactoryActors.hpp              ← enemy factory (existing)
```

## WGF Schema Extension

Add `enemies` array to existing WGF files:

```json
{
  "uuid": "a1b2c3d4-...",
  "name": "Enemy Wave Alpha",
  "difficulty": 3.0,
  "obstacles": [],
  "enemies": [
    {
      "type": "mermaid",
      "spawn_time": 2.0,
      "position": {"x": 100, "y": 200},
      "pattern": "SineHorizontal",
      "pattern_params": {
        "amplitude": {"x": 0, "y": 50},
        "frequency": {"x": 0, "y": 1},
        "base_dir": {"x": -1, "y": 0},
        "speed": 150
      }
    },
    {
      "type": "mermaid",
      "spawn_time": 2.5,
      "position": {"x": 100, "y": 400},
      "formation": {
        "type": "V-shape",
        "count": 3,
        "spacing": 60
      },
      "pattern": "Straight",
      "pattern_params": {
        "base_dir": {"x": -1, "y": 0},
        "speed": 100
      }
    }
  ]
}
```

### Enemy Spawn Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | ✅ | Enemy tag from `client/assets/data/*.json` |
| `spawn_time` | float | ✅ | Seconds after frame starts |
| `position` | Vec2f | ✅ | Spawn position relative to frame |
| `pattern` | string | ✅ | `PatternMovement::PatternType` name |
| `pattern_params` | object | ❌ | Pattern-specific parameters |
| `formation` | object | ❌ | Spawn multiple enemies in formation |

### Supported Patterns (from existing code)

| Pattern | Parameters | Behavior |
|---------|------------|----------|
| `Straight` | `base_dir`, `speed` | Move in straight line |
| `SineHorizontal` | `amplitude`, `frequency`, `base_dir`, `speed` | Move left with vertical sine wave |
| `SineVertical` | `amplitude`, `frequency`, `base_dir`, `speed` | Move up/down with horizontal sine wave |
| `ZigZagHorizontal` | `amplitude`, `frequency`, `base_dir`, `speed` | Sharp vertical zig-zag |
| `ZigZagVertical` | `amplitude`, `frequency`, `base_dir`, `speed` | Sharp horizontal zig-zag |
| `Wave` | `amplitude`, `frequency`, `base_dir`, `speed` | Combined wave motion |
| `Waypoints` | `waypoints[]`, `speed` | Follow predefined path |
| `FollowPlayer` | `speed` | Chase player |
| `Circular` | `speed`, `radius` | Orbit spawn point |

### Formation Types

| Formation | Description |
|-----------|-------------|
| `line` | Horizontal row |
| `column` | Vertical column |
| `V-shape` | Inverted V formation |
| `diamond` | Diamond/rhombus shape |
| `circle` | Circular arrangement |

## Data Structures

### Add to `WorldGenTypes.hpp`

```cpp
/**
 * @brief Pattern parameters for enemy movement.
 */
struct PatternParams {
    Vec2f amplitude = {0.0f, 0.0f};
    Vec2f frequency = {0.0f, 0.0f};
    Vec2f base_dir = {-1.0f, 0.0f};
    float speed = 100.0f;
    std::vector<Vec2f> waypoints;
    float radius = 100.0f;
};

/**
 * @brief Formation configuration for group spawns.
 */
struct FormationConfig {
    std::string type = "line";
    int count = 1;
    float spacing = 50.0f;
};

/**
 * @brief Enemy spawn definition within a WGF.
 */
struct EnemySpawn {
    std::string type;            ///< Enemy tag (e.g., "mermaid")
    float spawn_time = 0.0f;     ///< Seconds after frame starts
    Vec2f position;              ///< Spawn position
    std::string pattern;         ///< PatternType name
    PatternParams pattern_params;
    FormationConfig formation;
};

// Add to WGFDefinition:
struct WGFDefinition {
    // ... existing fields ...
    std::vector<EnemySpawn> enemies;  ///< Enemy spawn definitions
};
```

## Implementation Flow

### 1. Loading (WorldGenConfigLoader)

```cpp
// In LoadWGFFile(), after parsing obstacles:
if (json_data.contains("enemies")) {
    for (const auto& e : json_data["enemies"]) {
        EnemySpawn spawn;
        spawn.type = e.value("type", "mermaid");
        spawn.spawn_time = e.value("spawn_time", 0.0f);
        spawn.position = ParseVec2f(e["position"]);
        spawn.pattern = e.value("pattern", "Straight");
        
        if (e.contains("pattern_params")) {
            auto& pp = e["pattern_params"];
            spawn.pattern_params.amplitude = ParseVec2f(pp, "amplitude");
            spawn.pattern_params.frequency = ParseVec2f(pp, "frequency");
            spawn.pattern_params.base_dir = ParseVec2f(pp, "base_dir");
            spawn.pattern_params.speed = pp.value("speed", 100.0f);
        }
        
        if (e.contains("formation")) {
            auto& f = e["formation"];
            spawn.formation.type = f.value("type", "line");
            spawn.formation.count = f.value("count", 1);
            spawn.formation.spacing = f.value("spacing", 50.0f);
        }
        
        wgf.enemies.push_back(spawn);
    }
}
```

### 2. Runtime Spawning (Server)

```cpp
class FrameEnemyManager {
    const WGFDefinition* current_frame_ = nullptr;
    float frame_elapsed_ = 0.0f;
    std::set<size_t> spawned_indices_;

public:
    void SetFrame(const WGFDefinition* frame) {
        current_frame_ = frame;
        frame_elapsed_ = 0.0f;
        spawned_indices_.clear();
    }

    void Update(float dt, std::function<void(const EnemySpawn&)> spawn_fn) {
        if (!current_frame_) return;
        frame_elapsed_ += dt;

        for (size_t i = 0; i < current_frame_->enemies.size(); i++) {
            const auto& enemy = current_frame_->enemies[i];
            if (enemy.spawn_time <= frame_elapsed_ && 
                spawned_indices_.find(i) == spawned_indices_.end()) {
                spawn_fn(enemy);
                spawned_indices_.insert(i);
            }
        }
    }
};
```

### 3. Entity Creation (Client via FactoryActors)

```cpp
void SpawnEnemyFromConfig(
    Engine::registry& reg, 
    const EnemySpawn& spawn,
    float frame_x_offset) 
{
    auto& factory = FactoryActors::GetInstance();
    
    // Calculate formation positions
    auto positions = CalculateFormation(
        spawn.position, spawn.formation);
    
    for (const auto& pos : positions) {
        auto entity = reg.spawn_entity();
        
        // Use existing factory (loads from JSON config)
        factory.CreateActor(entity, reg, spawn.type);
        
        // Override position
        auto& transform = reg.GetComponent<Component::Transform>(entity);
        transform.x = pos.x + frame_x_offset;
        transform.y = pos.y;
        
        // Override pattern movement from WGF
        auto& pattern = reg.GetComponent<Component::PatternMovement>(entity);
        pattern.type = StringToPatternType(spawn.pattern);
        pattern.amplitude = {spawn.pattern_params.amplitude.x, 
                             spawn.pattern_params.amplitude.y};
        pattern.frequency = {spawn.pattern_params.frequency.x,
                             spawn.pattern_params.frequency.y};
        pattern.baseDir = {spawn.pattern_params.base_dir.x,
                           spawn.pattern_params.base_dir.y};
        pattern.baseSpeed = spawn.pattern_params.speed;
    }
}
```

## Integration with Existing Systems

### Uses Existing Code

| Component | File | Purpose |
|-----------|------|---------|
| `PatternMovement` | `GameplayComponents.hpp` | Enemy AI patterns |
| `FactoryActors` | `FactoryActors.hpp` | Enemy entity creation |
| `mermaid.json` | `assets/data/` | Enemy base stats |
| `TimedEvents` | `GameplayComponents.hpp` | Attack cooldowns |
| `FrameEvents` | `GameplayComponents.hpp` | Animation triggers |

### Data Flow

```
1. WGF JSON loaded → EnemySpawn structs stored
2. Frame selected by seed → Server tracks frame time
3. spawn_time reached → Server sends SPAWN_ENEMY packet
4. Client receives → FactoryActors creates entity
5. PatternMovement system → Handles AI movement
6. TimedEvents/FrameEvents → Handles shooting
```

## Example: Complete WGF with Enemies

```json
{
  "uuid": "b2c3d4e5-f6a7-4b8c-9d0e-1f2a3b4c5d6e",
  "name": "Mermaid Ambush",
  "difficulty": 4.0,
  "tags": ["enemies", "ambush", "medium"],
  "obstacles": [
    {
      "type": "decoration",
      "sprite": "images/debris_small.png",
      "position": {"x": 200, "y": 100},
      "size": {"width": 32, "height": 32}
    }
  ],
  "enemies": [
    {
      "type": "mermaid",
      "spawn_time": 1.0,
      "position": {"x": 850, "y": 300},
      "pattern": "SineHorizontal",
      "pattern_params": {
        "amplitude": {"x": 0, "y": 80},
        "frequency": {"x": 0, "y": 1.5},
        "base_dir": {"x": -1, "y": 0},
        "speed": 120
      }
    },
    {
      "type": "mermaid",
      "spawn_time": 3.0,
      "position": {"x": 850, "y": 200},
      "formation": {"type": "V-shape", "count": 3, "spacing": 50},
      "pattern": "Straight",
      "pattern_params": {
        "base_dir": {"x": -1, "y": 0},
        "speed": 100
      }
    }
  ],
  "background": {
    "layers": [
      {"sprite": "images/bg_stars.png", "parallax_factor": 0.3}
    ]
  }
}
```

## Testing

Add to `test_worldgen.cpp`:

```cpp
TEST_F(WorldGenTest, ParseEnemySpawns) {
    auto wgf = CreateWGFWithEnemies();
    ASSERT_EQ(wgf.enemies.size(), 2);
    
    EXPECT_EQ(wgf.enemies[0].type, "mermaid");
    EXPECT_FLOAT_EQ(wgf.enemies[0].spawn_time, 1.0f);
    EXPECT_EQ(wgf.enemies[0].pattern, "SineHorizontal");
    
    EXPECT_EQ(wgf.enemies[1].formation.count, 3);
}
```

## Advantages

1. **Reuses existing systems**: `PatternMovement`, `FactoryActors`, enemy JSON configs
2. **Data-driven**: Designers edit JSON, no code changes
3. **Deterministic**: Same seed + same frame = same enemy spawns
4. **Extensible**: Add new patterns by extending `PatternMovement::PatternType`
5. **Moddable**: Users can create custom WGFs with enemy waves
