# Lua Scripted Enemy Patterns

**Design Document** - Extending enemy AI with Lua scripts for complex behaviors

## Overview

While the existing `PatternMovement` component covers common patterns (sine wave, waypoints, etc.), Lua scripts enable complex, multi-phase behaviors for bosses and special enemies without recompiling C++ code.

## When to Use Lua vs PatternMovement

| Use Case | Recommendation |
|----------|----------------|
| Basic enemies (grunt, mermaid) | `PatternMovement` (existing C++) |
| Standard patterns (sine, zigzag, follow) | `PatternMovement` (existing C++) |
| Multi-phase boss fights | **Lua script** |
| Unique one-off behaviors | **Lua script** |
| User-created custom enemies | **Lua script** |
| Performance-critical spawns (100+ enemies) | `PatternMovement` (C++) |

## Architecture

```
server/assets/
├── worldgen/core/boss_level.wgf.json    ← references script
└── scripts/enemies/
    ├── boss_phase_pattern.lua           ← boss AI
    └── kamikaze_swarm.lua               ← swarm behavior

client/
├── include/components/
│   └── ScriptComponent.hpp              ← NEW: Lua state holder
└── engine/systems/
    └── LuaScriptSystem.cpp              ← NEW: script executor
```

## Dependencies

Add to `vcpkg.json`:
```json
{
  "dependencies": [
    "lua",
    "sol2"
  ]
}
```

Sol2 is a C++ header-only library providing clean Lua bindings.

## WGF Schema with Scripts

```json
{
  "uuid": "boss-uuid-here",
  "name": "Level 1 Boss",
  "enemies": [
    {
      "type": "boss_crab",
      "spawn_time": 0.0,
      "position": {"x": 700, "y": 300},
      "script": "scripts/enemies/boss_crab.lua",
      "script_params": {
        "phase1_duration": 15.0,
        "phase2_speed": 200.0,
        "rage_threshold": 0.3
      }
    }
  ]
}
```

## Lua Script API

### Script Structure

```lua
-- scripts/enemies/boss_crab.lua

-- Called once when enemy spawns
function on_spawn(entity, params)
    entity.phase = 1
    entity.phase_timer = 0
    entity.phase1_duration = params.phase1_duration or 15.0
    entity.rage_threshold = params.rage_threshold or 0.3
end

-- Called every frame
function update(entity, dt, params)
    entity.phase_timer = entity.phase_timer + dt
    
    -- Phase 1: Circle strafe
    if entity.phase == 1 then
        phase1_behavior(entity, dt)
        
        if entity.phase_timer > entity.phase1_duration then
            entity.phase = 2
            entity.phase_timer = 0
            play_sound("boss_roar")
        end
    
    -- Phase 2: Aggressive chase
    elseif entity.phase == 2 then
        phase2_behavior(entity, dt, params)
        
        -- Enter rage mode at low health
        if entity.health < entity.max_health * entity.rage_threshold then
            entity.phase = 3
            set_sprite(entity, "boss_crab_rage")
        end
    
    -- Phase 3: Rage mode
    elseif entity.phase == 3 then
        rage_behavior(entity, dt)
    end
end

-- Called when entity takes damage
function on_hit(entity, damage, source)
    flash_sprite(entity, 0.1)
    
    if entity.phase == 1 then
        -- Briefly pause movement when hit
        entity.stun_timer = 0.2
    end
end

-- Called when entity dies
function on_death(entity)
    spawn_explosion(entity.x, entity.y, "large")
    spawn_powerup(entity.x, entity.y, "bomb")
    add_score(5000)
end

-- Phase behaviors
function phase1_behavior(entity, dt)
    -- Circle around center of screen
    local center_x, center_y = 600, 300
    local radius = 150
    local speed = 2.0
    
    entity.angle = (entity.angle or 0) + speed * dt
    entity.vx = -math.sin(entity.angle) * radius * speed
    entity.vy = math.cos(entity.angle) * radius * speed
    
    -- Shoot at player every 2 seconds
    if entity.shoot_timer <= 0 then
        local px, py = get_player_pos()
        local dx, dy = normalize(px - entity.x, py - entity.y)
        spawn_bullet(entity.x, entity.y, dx, dy, 300)
        entity.shoot_timer = 2.0
    else
        entity.shoot_timer = entity.shoot_timer - dt
    end
end

function phase2_behavior(entity, dt, params)
    -- Chase player
    local px, py = get_player_pos()
    local dx, dy = normalize(px - entity.x, py - entity.y)
    
    entity.vx = dx * params.phase2_speed
    entity.vy = dy * params.phase2_speed
    
    -- Rapid fire
    if entity.shoot_timer <= 0 then
        spawn_bullet_spread(entity.x, entity.y, 5, 200)
        entity.shoot_timer = 0.5
    else
        entity.shoot_timer = entity.shoot_timer - dt
    end
end

function rage_behavior(entity, dt)
    -- Erratic movement
    entity.rage_timer = (entity.rage_timer or 0) + dt
    local speed = 350
    
    entity.vx = math.cos(entity.rage_timer * 5) * speed
    entity.vy = math.sin(entity.rage_timer * 3) * speed
    
    -- Constant bullet spray
    if entity.shoot_timer <= 0 then
        spawn_bullet_circle(entity.x, entity.y, 12, 250)
        entity.shoot_timer = 0.3
    else
        entity.shoot_timer = entity.shoot_timer - dt
    end
end

-- Helper
function normalize(x, y)
    local len = math.sqrt(x*x + y*y)
    if len > 0 then
        return x/len, y/len
    end
    return 0, 0
end
```

### Available Functions (C++ → Lua)

**Entity State:**
```lua
entity.x, entity.y          -- Position (read/write)
entity.vx, entity.vy        -- Velocity (read/write)
entity.health               -- Current health (read)
entity.max_health           -- Maximum health (read)
entity.rotation             -- Rotation degrees (read/write)
```

**Game Queries:**
```lua
get_player_pos()            -- Returns px, py
get_player_count()          -- Number of active players
get_nearest_player(x, y)    -- Returns player_id, px, py
is_player_alive(id)         -- Check if player exists
get_frame_time()            -- Time since frame started
```

**Actions:**
```lua
spawn_bullet(x, y, dx, dy, speed)
spawn_bullet_spread(x, y, count, speed)
spawn_bullet_circle(x, y, count, speed)
spawn_enemy(type, x, y)
spawn_powerup(x, y, type)
spawn_explosion(x, y, size)
```

**Audio/Visual:**
```lua
play_sound(sound_id)
set_sprite(entity, sprite_name)
set_animation(entity, anim_name)
flash_sprite(entity, duration)
shake_screen(intensity, duration)
```

**Score/Game:**
```lua
add_score(points)
set_boss_health_bar(entity)
hide_boss_health_bar()
```

## C++ Implementation

### ScriptComponent

```cpp
// client/include/components/ScriptComponent.hpp
#pragma once
#include <sol/sol.hpp>
#include <string>
#include <map>

namespace Rtype::Client::Component {

struct LuaScript {
    sol::state lua;
    sol::function on_spawn;
    sol::function update;
    sol::function on_hit;
    sol::function on_death;
    
    std::string script_path;
    std::map<std::string, float> params;
    
    // Script-local state (accessible in Lua as entity.*)
    float phase = 0;
    float phase_timer = 0;
    float shoot_timer = 0;
    float angle = 0;
    // ... other script variables stored in Lua table
};

}  // namespace Rtype::Client::Component
```

### LuaScriptSystem

```cpp
// client/engine/systems/LuaScriptSystem.cpp
#include <sol/sol.hpp>

namespace Rtype::Client {

class LuaScriptSystem {
public:
    void InitializeLuaBindings(sol::state& lua, Engine::registry& reg) {
        // Bind game functions
        lua.set_function("get_player_pos", [&reg]() {
            // Find player entity, return position
            for (auto [e, player, transform] : 
                 reg.view<Component::PlayerTag, Component::Transform>()) {
                return std::make_tuple(transform.x, transform.y);
            }
            return std::make_tuple(400.0f, 300.0f);
        });
        
        lua.set_function("spawn_bullet", 
            [&reg](float x, float y, float dx, float dy, float speed) {
                CreateEnemyBullet(reg, x, y, dx, dy, speed);
            });
        
        lua.set_function("play_sound", [](const std::string& id) {
            AudioManager::GetInstance().PlaySound(id);
        });
        
        // ... bind other functions
    }
    
    void LoadScript(Component::LuaScript& script, const std::string& path) {
        script.lua.open_libraries(sol::lib::base, sol::lib::math);
        InitializeLuaBindings(script.lua, registry_);
        
        script.lua.script_file(path);
        
        script.on_spawn = script.lua["on_spawn"];
        script.update = script.lua["update"];
        script.on_hit = script.lua["on_hit"];
        script.on_death = script.lua["on_death"];
    }
    
    void SpawnEntity(Engine::entity entity, Component::LuaScript& script) {
        if (script.on_spawn.valid()) {
            sol::table entity_table = CreateEntityTable(script.lua, entity);
            sol::table params_table = CreateParamsTable(script.lua, script.params);
            script.on_spawn(entity_table, params_table);
        }
    }
    
    void Update(float dt) {
        for (auto [entity, script, transform, velocity] :
             registry_.view<Component::LuaScript, 
                           Component::Transform, 
                           Component::Velocity>()) {
            
            if (script.update.valid()) {
                // Create entity table with current state
                sol::table entity_table = script.lua.create_table();
                entity_table["x"] = transform.x;
                entity_table["y"] = transform.y;
                entity_table["vx"] = velocity.vx;
                entity_table["vy"] = velocity.vy;
                entity_table["health"] = GetHealth(entity);
                // Copy script-local state
                
                sol::table params = CreateParamsTable(script.lua, script.params);
                
                // Call Lua update
                script.update(entity_table, dt, params);
                
                // Read back modified values
                velocity.vx = entity_table["vx"];
                velocity.vy = entity_table["vy"];
                transform.rotation = entity_table.get_or("rotation", 0.0f);
            }
        }
    }
    
    void OnEntityHit(Engine::entity entity, int damage, int source) {
        if (registry_.has_component<Component::LuaScript>(entity)) {
            auto& script = registry_.GetComponent<Component::LuaScript>(entity);
            if (script.on_hit.valid()) {
                sol::table entity_table = CreateEntityTable(script.lua, entity);
                script.on_hit(entity_table, damage, source);
            }
        }
    }
};

}  // namespace Rtype::Client
```

### Data Structure Extension

```cpp
// Add to WorldGenTypes.hpp
struct EnemySpawn {
    std::string type;
    float spawn_time = 0.0f;
    Vec2f position;
    
    // Standard pattern (mutually exclusive with script)
    std::string pattern;
    PatternParams pattern_params;
    
    // Lua script (for complex behaviors)
    std::string script_path;
    std::map<std::string, float> script_params;
    
    bool HasScript() const { return !script_path.empty(); }
};
```

### Entity Creation

```cpp
void SpawnEnemyFromConfig(
    Engine::registry& reg, 
    const EnemySpawn& spawn,
    float frame_x_offset) 
{
    auto& factory = FactoryActors::GetInstance();
    auto entity = reg.spawn_entity();
    
    // Create base enemy (stats, sprite, hitbox from JSON)
    factory.CreateActor(entity, reg, spawn.type);
    
    // Set position
    auto& transform = reg.GetComponent<Component::Transform>(entity);
    transform.x = spawn.position.x + frame_x_offset;
    transform.y = spawn.position.y;
    
    if (spawn.HasScript()) {
        // Use Lua script for AI
        auto& script = reg.AddComponent<Component::LuaScript>(entity);
        script.script_path = spawn.script_path;
        script.params = spawn.script_params;
        
        LuaScriptSystem::GetInstance().LoadScript(
            script, "assets/" + spawn.script_path);
        LuaScriptSystem::GetInstance().SpawnEntity(entity, script);
        
        // Remove PatternMovement if present (script handles movement)
        reg.RemoveComponent<Component::PatternMovement>(entity);
    } else {
        // Use standard PatternMovement
        auto& pattern = reg.GetComponent<Component::PatternMovement>(entity);
        pattern.type = StringToPatternType(spawn.pattern);
        pattern.amplitude = ToSfVector(spawn.pattern_params.amplitude);
        pattern.frequency = ToSfVector(spawn.pattern_params.frequency);
        pattern.baseDir = ToSfVector(spawn.pattern_params.base_dir);
        pattern.baseSpeed = spawn.pattern_params.speed;
    }
}
```

## Example: Swarm Script

```lua
-- scripts/enemies/kamikaze_swarm.lua
-- Multiple small enemies that coordinate attacks

local swarm_members = {}

function on_spawn(entity, params)
    entity.swarm_id = params.swarm_id or 0
    entity.formation_offset = params.formation_offset or 0
    entity.attack_delay = entity.formation_offset * 0.5
    entity.state = "approach"
    
    table.insert(swarm_members, entity)
end

function update(entity, dt, params)
    if entity.state == "approach" then
        -- Move towards formation point
        local target_x = 500
        local target_y = 100 + entity.formation_offset * 60
        
        local dx, dy = normalize(target_x - entity.x, target_y - entity.y)
        entity.vx = dx * 150
        entity.vy = dy * 150
        
        -- Reached formation?
        if math.abs(entity.x - target_x) < 20 and 
           math.abs(entity.y - target_y) < 20 then
            entity.state = "wait"
            entity.wait_timer = entity.attack_delay
        end
    
    elseif entity.state == "wait" then
        entity.vx = 0
        entity.vy = 0
        entity.wait_timer = entity.wait_timer - dt
        
        if entity.wait_timer <= 0 then
            entity.state = "attack"
            play_sound("kamikaze_scream")
        end
    
    elseif entity.state == "attack" then
        -- Dive at player
        local px, py = get_player_pos()
        local dx, dy = normalize(px - entity.x, py - entity.y)
        entity.vx = dx * 400
        entity.vy = dy * 400
    end
end

function on_death(entity)
    spawn_explosion(entity.x, entity.y, "small")
    
    -- Remove from swarm tracking
    for i, member in ipairs(swarm_members) do
        if member == entity then
            table.remove(swarm_members, i)
            break
        end
    end
end
```

## Performance Considerations

| Aspect | Impact | Mitigation |
|--------|--------|------------|
| Lua overhead | ~0.1ms per entity per frame | Limit to bosses/special enemies |
| Memory | ~50KB per Lua state | Share state for swarms |
| GC pauses | Occasional 1-2ms spikes | Use incremental GC |

**Recommendation:** Use Lua for ≤10 simultaneous scripted enemies. Use `PatternMovement` for masses of simple enemies.

## Testing Scripts

```lua
-- test_boss.lua - Run with: lua test_boss.lua

-- Mock game functions
function get_player_pos() return 100, 300 end
function spawn_bullet() end
function play_sound() end

-- Load script
dofile("scripts/enemies/boss_crab.lua")

-- Create mock entity
local entity = {
    x = 700, y = 300,
    vx = 0, vy = 0,
    health = 100,
    max_health = 100,
    shoot_timer = 0
}

local params = {
    phase1_duration = 15.0,
    phase2_speed = 200.0,
    rage_threshold = 0.3
}

-- Test
on_spawn(entity, params)
assert(entity.phase == 1, "Should start in phase 1")

for i = 1, 100 do
    update(entity, 0.16, params)
end

print("Boss script tests passed!")
```

## Summary

| Feature | PatternMovement (C++) | Lua Scripts |
|---------|----------------------|-------------|
| Setup | Add component | Load script file |
| Patterns | 9 predefined | Unlimited |
| Multi-phase | ❌ | ✅ |
| State machine | ❌ | ✅ |
| Custom shooting | Via FrameEvents | Direct API |
| Hot reload | ❌ | ✅ (dev only) |
| Performance | Fastest | ~10% overhead |
| User mods | Limited | Full flexibility |

**Hybrid approach**: Use `PatternMovement` for common enemies (95%), Lua for bosses and unique encounters (5%).
