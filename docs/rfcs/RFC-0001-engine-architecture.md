# RFC-0001: Engine Architecture


---

## Overview

This document describes the R-Type game engine architecture based on the **Entity-Component-System (ECS)** pattern. The engine provides a data-oriented approach that separates game data from behavior, enabling high performance, modularity, and maintainability.

---

## What is ECS?

**Entity-Component-System (ECS)** is an architectural pattern commonly used in game development that separates:
- **Data** (Components) from **Logic** (Systems)
- **Identity** (Entities) from both

### Traditional OOP vs ECS

**Traditional Object-Oriented Approach:**
```
GameObject
  ├─ Position (x, y)
  ├─ Sprite (texture)
  ├─ Health (hp)
  └─ update() method

Enemy extends GameObject
  ├─ AI logic
  └─ override update()

Player extends GameObject
  ├─ Input handling
  └─ override update()
```

**Problems:**
- Deep inheritance hierarchies
- Tight coupling between data and behavior
- Poor memory layout (objects scattered in memory)
- Difficult to compose new behaviors

**ECS Approach:**
```
Entity: Just an ID (e.g., 42)

Components (Pure Data):
  - Position[42] = {x: 100, y: 200}
  - Sprite[42] = {texture: "player.png"}
  - Health[42] = {hp: 100}

Systems (Pure Logic):
  - MovementSystem: reads Position + Velocity, updates Position
  - RenderSystem: reads Position + Sprite, draws to screen
  - CollisionSystem: reads Position + Collider, detects collisions
```

**Benefits:**
- ✅ Composition over inheritance
- ✅ Data-oriented (cache-friendly)
- ✅ Decoupled subsystems
- ✅ Easy to add/remove behaviors at runtime

---

## Core Concepts

### 1. Entity

An **Entity** is simply a unique identifier (ID) representing a game object.

```
┌─────────────────────────────────────┐
│         Entity                      │
│  "Just an ID number"                │
│                                     │
│  Entity ID: 0  →  Player            │
│  Entity ID: 1  →  Enemy             │
│  Entity ID: 2  →  Missile           │
│  Entity ID: 3  →  Powerup           │
└─────────────────────────────────────┘
```

**Key Points:**
- No data or behavior attached
- Lightweight (just a number)
- Acts as an index into component arrays

**Implementation:**
```cpp
class entity {
private:
    size_t id_;  // Just an ID!
    
    explicit entity(size_t id) : id_(id) {}
    friend class registry;  // Only registry can create entities
    
public:
    size_t get_id() const { return id_; }
};
```

**Pseudo-code:**
```
entity = new_id()
// Entity 42 created (but has no data yet)
```

---

### 2. Component

A **Component** is a plain data structure representing a single aspect of an entity.

```
┌──────────────────────────────────────────────┐
│            Components                        │
│      "Pure Data Structures"                  │
│                                              │
│  struct Position { float x, y; }             │
│  struct Velocity { float dx, dy; }           │
│  struct Sprite { string texture; }           │
│  struct Health { int hp, max_hp; }           │
│  struct Controllable { int player_id; }      │
└──────────────────────────────────────────────┘
```

**Key Points:**
- Only contains data (no methods)
- Plain-Old-Data (POD) structures
- Represents a single characteristic

**Examples:**
```cpp
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

struct Sprite {
    std::string texture_path;
    int frame;
};
```

**Pseudo-code:**
```
// Entity 42 gets components
entity[42].add(Position{x: 100, y: 200})
entity[42].add(Velocity{dx: 5, dy: 0})
entity[42].add(Sprite{texture: "player.png"})

// Entity 42 now has: Position + Velocity + Sprite
```

---

### 3. Sparse Array

A **Sparse Array** is a specialized container that stores components efficiently using entity IDs as indices.

```
Sparse Array Structure:

         Entity IDs (indices)
         ↓  ↓  ↓  ↓  ↓  ↓
       ┌───┬───┬───┬───┬───┬───┐
       │ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │
       ├───┼───┼───┼───┼───┼───┤
Pos →  │ P │∅ │ P │∅ │ P │∅ │
       └───┴───┴───┴───┴───┴───┘
         ↑       ↑       ↑
      Entity 0,2,4 have Position

       ┌───┬───┬───┬───┬───┬───┐
       │ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │
       ├───┼───┼───┼───┼───┼───┤
Vel →  │∅ │ V │∅ │ V │ V │∅ │
       └───┴───┴───┴───┴───┴───┘
         ↑       ↑   ↑
      Entity 1,3,4 have Velocity

Key: P = Position data
     V = Velocity data
     ∅ = Empty (std::nullopt)
```

**Key Points:**
- Direct access: `array[entity_id]` → O(1)
- Stores `std::optional<Component>` (empty if entity doesn't have component)
- Memory efficient for moderate entity counts

**Implementation:**
```cpp
template <typename Component>
class sparse_array {
    std::vector<std::optional<Component>> _data;
    
public:
    // O(1) access by entity ID (no bounds checking - undefined behavior if out of bounds)
    std::optional<Component>& operator[](size_t entity_id) {
        return _data[entity_id];
    }
    
    // Insert component at entity position (auto-resizes if needed)
    std::optional<Component>& insert_at(size_t entity_id, Component&& component) {
        if (entity_id >= _data.size()) {
            _data.resize(entity_id + 1);
        }
        _data[entity_id] = std::move(component);
        return _data[entity_id];
    }
    
    // Check if entity has this component
    bool has(size_t entity_id) const {
        return entity_id < _data.size() && _data[entity_id].has_value();
    }
    
    // Remove component
    void erase(size_t entity_id) {
        if (entity_id < _data.size()) {
            _data[entity_id] = std::nullopt;
        }
    }
};
```

**Pseudo-code:**
```
sparse_array<Position> positions

positions[0] = Position{100, 200}  // Entity 0 has position
positions[2] = Position{300, 400}  // Entity 2 has position
// positions[1] is empty (std::nullopt)

if positions.has(2):
    pos = positions[2]
    print(pos.x, pos.y)  // 300, 400
```

---

### 4. Registry

The **Registry** is the central manager that coordinates entities, components, and systems.

```
┌────────────────────────────────────────────────────────┐
│                     REGISTRY                           │
│            "The Central Coordinator"                   │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Entity Management:                                    │
│    _next_entity: 5                                     │
│    _dead_entities: [2, 7, 9]  (reusable IDs)         │
│                                                        │
│  Component Storage:                                    │
│    Position     → sparse_array<Position>              │
│    Velocity     → sparse_array<Velocity>              │
│    Sprite       → sparse_array<Sprite>                │
│    Health       → sparse_array<Health>                │
│                                                        │
│  Systems:                                              │
│    [MovementSystem, RenderSystem, CollisionSystem]    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

**Key Points:**
- Creates and destroys entities
- Manages all component arrays
- Registers and runs systems
- Provides API for entity-component operations

**Implementation:**
```cpp
class registry {
    // Entity lifecycle
    size_t _next_entity = 0;
    std::vector<size_t> _dead_entities;
    
    // Component storage (type-erased)
    std::unordered_map<std::type_index, 
                       std::unique_ptr<components_holder_base>> _components_arrays;
    
    // Systems
    std::vector<std::function<void(registry&)>> _systems;
    
public:
    // Create new entity
    entity spawn_entity() {
        size_t id;
        if (!_dead_entities.empty()) {
            id = _dead_entities.back();
            _dead_entities.pop_back();
        } else {
            id = _next_entity++;
        }
        return entity(id);
    }
    
    // Register component type
    template <typename Component>
    sparse_array<Component>& register_component() {
        auto type = std::type_index(typeid(Component));
        if (_components_arrays.count(type)) {
            throw std::runtime_error("Component already registered in registry");
        }
        _components_arrays[type] = std::make_unique<components_holder<Component>>();
        // Also registers cleanup function for entity destruction
        return static_cast<components_holder<Component>*>(
            _components_arrays[type].get()
        )->arr;
    }
    
    // Add component to entity
    template <typename Component>
    std::optional<Component>& add_component(entity e, Component&& comp) {
        return get_components<Component>().insert_at(e.get_id(), std::move(comp));
    }
    
    // Get component array
    template <typename Component>
    sparse_array<Component>& get_components() {
        auto type = std::type_index(typeid(Component));
        return *static_cast<components_holder<Component>*>(
            _components_arrays[type].get()
        )->arr;
    }
    
    // Register system
    template <typename... Components, typename Function>
    void add_system(Function&& f) {
        _systems.push_back([f](registry& reg) {
            f(reg, reg.get_components<Components>()...);
        });
    }
    
    // Run all systems
    void run_systems() {
        for (auto& system : _systems) {
            system(*this);
        }
    }
};

// Type-erased base class
struct components_holder_base {
    virtual ~components_holder_base() = default;
};

// Template wrapper for each component type
template <typename T>
struct components_holder : components_holder_base {
    sparse_array<T> arr;
};
```

**Pseudo-code:**
```
registry = new Registry()

// 1. Register component types
registry.register_component<Position>()
registry.register_component<Velocity>()

// 2. Create entities
player = registry.spawn_entity()
enemy = registry.spawn_entity()

// 3. Add components
registry.add_component(player, Position{100, 200})
registry.add_component(player, Velocity{5, 0})
registry.add_component(enemy, Position{300, 400})

// 4. Register systems
registry.add_system<Position, Velocity>(movement_system)
registry.add_system<Position, Sprite>(render_system)

// 5. Game loop
while running:
    registry.run_systems()  // Executes all systems
```

---

### 5. System

A **System** is a function that operates on entities with specific components.

```
┌──────────────────────────────────────────────────────┐
│                  SYSTEM                              │
│           "Pure Logic Functions"                     │
│                                                      │
│  Input:  Component arrays                            │
│  Output: Modified component data                     │
│                                                      │
│  Example: MovementSystem                             │
│    Reads:  Position[], Velocity[]                    │
│    Writes: Position[]                                │
│    Logic:  position += velocity * delta_time         │
└──────────────────────────────────────────────────────┘
```

**Key Points:**
- Stateless (no internal state)
- Operates on component arrays
- Declarative dependencies (which components needed)

**Implementation:**
```cpp
// Movement System: Updates position based on velocity
void movement_system(registry& reg, 
                     sparse_array<Position>& positions,
                     sparse_array<Velocity>& velocities) {
    for (size_t i = 0; i < positions.size(); ++i) {
        // Only process entities with BOTH Position AND Velocity
        if (positions[i] && velocities[i]) {
            positions[i]->x += velocities[i]->dx;
            positions[i]->y += velocities[i]->dy;
        }
    }
}

// Render System: Draws sprites at positions
void render_system(registry& reg,
                   sparse_array<Position>& positions,
                   sparse_array<Sprite>& sprites) {
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i] && sprites[i]) {
            draw_sprite(sprites[i]->texture, 
                       positions[i]->x, 
                       positions[i]->y);
        }
    }
}

// Collision System: Detects collisions
void collision_system(registry& reg,
                      sparse_array<Position>& positions,
                      sparse_array<Collider>& colliders) {
    for (size_t i = 0; i < positions.size(); ++i) {
        if (!positions[i] || !colliders[i]) continue;
        
        for (size_t j = i + 1; j < positions.size(); ++j) {
            if (!positions[j] || !colliders[j]) continue;
            
            if (check_aabb_collision(positions[i], colliders[i],
                                     positions[j], colliders[j])) {
                handle_collision(reg, i, j);
            }
        }
    }
}
```

**Pseudo-code:**
```
function movement_system(registry):
    positions = registry.get_components<Position>()
    velocities = registry.get_components<Velocity>()
    
    for entity_id in 0..max_entities:
        if positions.has(entity_id) AND velocities.has(entity_id):
            pos = positions[entity_id]
            vel = velocities[entity_id]
            
            pos.x += vel.dx
            pos.y += vel.dy
```

---

## Complete Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                          GAME LOOP                                  │
│                                                                     │
│  while running:                                                     │
│      handle_input()                                                 │
│      registry.run_systems()    ← Execute all systems                │
│      render()                                                       │
└─────────────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────────────┐
│                          REGISTRY                                   │
│                    (Central Coordinator)                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Entities:        [0, 1, 2, 3, 4, ...]                              │
│  Dead Entities:   [2, 7]  ← Recycled IDs                            │
│                                                                     │
│  Components:                                                        │
│  ┌──────────────────────────────────────────────────────┐           │
│  │ sparse_array<Position>:                              │           │
│  │   [0]: {x: 100, y: 200}                              │           │
│  │   [1]: {x: 300, y: 400}                              │           │
│  │   [3]: {x: 150, y: 250}                              │           │
│  └──────────────────────────────────────────────────────┘           │
│  ┌──────────────────────────────────────────────────────┐           │
│  │ sparse_array<Velocity>:                              │           │
│  │   [0]: {dx: 5, dy: 0}                                │           │
│  │   [1]: {dx: -3, dy: 2}                               │           │
│  └──────────────────────────────────────────────────────┘           │
│  ┌──────────────────────────────────────────────────────┐           │
│  │ sparse_array<Sprite>:                                │           │
│  │   [0]: {texture: "player.png"}                       │           │
│  │   [3]: {texture: "enemy.png"}                        │           │
│  └──────────────────────────────────────────────────────┘           │
│                                                                     │
│  Systems (executed in order):                                       │
│  1. InputSystem      → Updates Velocity based on player input       │
│  2. MovementSystem   → Updates Position based on Velocity           │
│  3. CollisionSystem  → Detects entity overlaps                      │
│  4. RenderSystem     → Draws Sprites at Positions                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────────────┐
│                    ENTITY COMPOSITION                               │
│                                                                     │
│  Entity 0 (Player):                                                 │
│    ├─ Position[0]  = {100, 200}                                     │
│    ├─ Velocity[0]  = {5, 0}                                         │
│    ├─ Sprite[0]    = {"player.png"}                                 │
│    └─ Health[0]    = {hp: 100, max: 100}                            │
│                                                                     │
│  Entity 1 (Enemy):                                                  │
│    ├─ Position[1]  = {300, 400}                                     │
│    ├─ Velocity[1]  = {-3, 2}                                        │
│    └─ Sprite[1]    = {"enemy.png"}                                  │
│                                                                     │
│  Entity 3 (Static Obstacle):                                        │
│    ├─ Position[3]  = {150, 250}                                     │
│    └─ Sprite[3]    = {"obstacle.png"}                               │
│    (No Velocity = doesn't move)                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Data Flow Example

### Scenario: Player Movement

```
1. INPUT
   ┌──────────────────────┐
   │ User presses RIGHT   │
   └──────────────────────┘
           ↓
2. INPUT SYSTEM
   ┌───────────────────────────────────────┐
   │ InputSystem:                          │
   │   player = entity 0                   │
   │   if key_pressed(RIGHT):              │
   │      velocities[0].dx = 5             │
   └───────────────────────────────────────┘
           ↓
3. MOVEMENT SYSTEM
   ┌───────────────────────────────────────┐
   │ MovementSystem:                       │
   │   positions[0].x += velocities[0].dx  │
   │   positions[0].x = 100 + 5 = 105      │
   └───────────────────────────────────────┘
           ↓
4. RENDER SYSTEM
   ┌───────────────────────────────────────┐
   │ RenderSystem:                         │
   │   draw(sprites[0], positions[0])      │
   │   → Draw "player.png" at (105, 200)   │
   └───────────────────────────────────────┘
```

---

## Memory Layout: Why ECS is Fast

### Traditional OOP (Poor Cache Locality)

```
Memory Layout:

GameObject[0] at 0x1000:
  ├─ x: 100.0
  ├─ y: 200.0
  ├─ sprite: "player.png"  ← Not needed for movement!
  ├─ health: 100           ← Not needed for movement!
  └─ vtable pointer        ← Overhead

GameObject[1] at 0x2500:  ← Far away in memory!
  ├─ x: 300.0
  ├─ y: 400.0
  ├─ sprite: "enemy.png"
  └─ ai_data: {...}

Problem: Loading GameObject[0] brings sprite/health into cache
         but movement only needs x, y!
         → Wasted cache lines
```

### ECS (Excellent Cache Locality)

```
Memory Layout:

sparse_array<Position> at 0x1000:
  [0] = {x: 100.0, y: 200.0}  ← 8 bytes
  [1] = {x: 300.0, y: 400.0}  ← 8 bytes
  [2] = {x: 150.0, y: 250.0}  ← 8 bytes
  ...
  All contiguous in memory!

sparse_array<Sprite> at 0x5000:  ← Separate array
  [0] = {"player.png"}
  [1] = {"enemy.png"}
  ...

Benefit: MovementSystem only touches Position array
         → All position data tightly packed
         → Great cache utilization
         → SIMD vectorization possible
```

---

## Practical Usage Example

```cpp
#include "registry.hpp"

// 1. Define components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Sprite { std::string texture; };
struct Health { int hp; };

int main() {
    // 2. Create registry
    Engine::registry reg;
    
    // 3. Register component types (returns sparse_array reference)
    auto& positions = reg.register_component<Position>();
    auto& velocities = reg.register_component<Velocity>();
    auto& sprites = reg.register_component<Sprite>();
    auto& healths = reg.register_component<Health>();
    
    // 4. Create entities
    auto player = reg.spawn_entity();
    auto enemy1 = reg.spawn_entity();
    auto enemy2 = reg.spawn_entity();
    
    // 5. Compose entities with components
    reg.add_component(player, Position{100.0f, 200.0f});
    reg.add_component(player, Velocity{0.0f, 0.0f});
    reg.add_component(player, Sprite{"player.png"});
    reg.add_component(player, Health{100});
    
    reg.add_component(enemy1, Position{300.0f, 400.0f});
    reg.add_component(enemy1, Velocity{-2.0f, 0.0f});
    reg.add_component(enemy1, Sprite{"enemy.png"});
    
    reg.add_component(enemy2, Position{500.0f, 300.0f});
    reg.add_component(enemy2, Velocity{-3.0f, 1.0f});
    reg.add_component(enemy2, Sprite{"enemy.png"});
    
    // 6. Register systems
    reg.add_system<Position, Velocity>(
        [](auto& reg, auto& positions, auto& velocities) {
            // Movement system
            for (size_t i = 0; i < positions.size(); ++i) {
                if (positions[i] && velocities[i]) {
                    positions[i]->x += velocities[i]->dx;
                    positions[i]->y += velocities[i]->dy;
                }
            }
        }
    );
    
    reg.add_system<Position, Sprite>(
        [](auto& reg, auto& positions, auto& sprites) {
            // Render system
            for (size_t i = 0; i < positions.size(); ++i) {
                if (positions[i] && sprites[i]) {
                    draw_sprite(sprites[i]->texture, 
                              positions[i]->x, 
                              positions[i]->y);
                }
            }
        }
    );
    
    // 7. Game loop
    while (game_running) {
        handle_input(reg);
        reg.run_systems();  // Execute all systems
        present_frame();
    }
    
    return 0;
}
```

---

## Key Advantages of ECS

### 1. Composition Over Inheritance
```
Traditional:
  Player extends GameObject extends Entity
  → Rigid hierarchy

ECS:
  Entity + Position + Velocity + Sprite + Controllable
  → Flexible composition
  → Add/remove behaviors at runtime
```

### 2. Data-Oriented Design
```
Systems iterate over contiguous arrays
→ Cache-friendly
→ SIMD vectorization
→ High performance
```

### 3. Decoupling
```
MovementSystem doesn't know about RenderSystem
RenderSystem doesn't know about CollisionSystem
→ Each system is independent
→ Easy to test
→ Easy to parallelize
```

### 4. Reusability
```
Same Position component used by:
  - MovementSystem
  - RenderSystem
  - CollisionSystem
  - NetworkSystem
```

---

## Summary

The R-Type engine implements a pure ECS architecture where:

| Concept | Purpose | Example |
|---------|---------|---------|
| **Entity** | Unique identifier | Player ID: 0 |
| **Component** | Pure data | `Position{x: 100, y: 200}` |
| **Sparse Array** | Component storage | `positions[0]`, `positions[1]`, ... |
| **Registry** | Central manager | Creates entities, stores components, runs systems |
| **System** | Game logic | Movement, Rendering, Collision |

**Philosophy:**
- Entities are **what** (ID)
- Components are **data** (attributes)
- Systems are **how** (behavior)

This separation enables the engine to be fast, flexible, and maintainable.

---

## References

- [Entity Component System (Wikipedia)](https://en.wikipedia.org/wiki/Entity_component_system)
- [Data-Oriented Design](https://www.dataorienteddesign.com/dodbook/)
- [EnTT Library](https://github.com/skypjack/entt) - Popular C++ ECS
- R-Type Project: `/engine/include/` (implementation details)
