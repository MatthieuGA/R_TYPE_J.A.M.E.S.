#include <gtest/gtest.h>
#include "include/registry.hpp"
#include "include/entity.hpp"
#include <string>

// Test components
struct Position {
    float x;
    float y;
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Velocity {
    float dx;
    float dy;
    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
};

struct Health {
    int hp;
    explicit Health(int hp = 100) : hp(hp) {}
};

struct Name {
    std::string value;
    explicit Name(std::string const &v = "") : value(v) {}
};

// ============================================================================
// COMPONENT REGISTRATION TESTS
// ============================================================================

TEST(RegistryTest, RegisterComponent) {
    Engine::registry reg;
    
    auto &positions = reg.register_component<Position>();
    
    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, RegisterMultipleComponents) {
    Engine::registry reg;
    
    auto &positions = reg.register_component<Position>();
    auto &velocities = reg.register_component<Velocity>();
    auto &healths = reg.register_component<Health>();
    
    EXPECT_EQ(positions.size(), 0);
    EXPECT_EQ(velocities.size(), 0);
    EXPECT_EQ(healths.size(), 0);
}

TEST(RegistryTest, RegisterComponentTwiceThrows) {
    Engine::registry reg;
    
    reg.register_component<Position>();
    
    EXPECT_THROW(reg.register_component<Position>(), std::runtime_error);
}

TEST(RegistryTest, GetComponents) {
    Engine::registry reg;
    
    reg.register_component<Position>();
    auto &positions = reg.get_components<Position>();
    
    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, GetComponentsConst) {
    Engine::registry reg;
    
    reg.register_component<Position>();
    const auto &const_reg = reg;
    const auto &positions = const_reg.get_components<Position>();
    
    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, GetComponentsNotRegisteredThrows) {
    Engine::registry reg;
    
    EXPECT_THROW(reg.get_components<Position>(), std::runtime_error);
}

// ============================================================================
// ENTITY MANAGEMENT TESTS
// ============================================================================

TEST(RegistryTest, SpawnEntity) {
    Engine::registry reg;
    
    auto entity = reg.spawn_entity();
    
    EXPECT_EQ(entity.getId(), 0);
}

TEST(RegistryTest, SpawnMultipleEntities) {
    Engine::registry reg;
    
    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    auto e3 = reg.spawn_entity();
    
    EXPECT_EQ(e1.getId(), 0);
    EXPECT_EQ(e2.getId(), 1);
    EXPECT_EQ(e3.getId(), 2);
}

TEST(RegistryTest, EntityFromIndex) {
    Engine::registry reg;
    
    auto entity = reg.entity_from_index(42);
    
    EXPECT_EQ(entity.getId(), 42);
}

TEST(RegistryTest, KillEntity) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{10.0f, 20.0f});
    
    EXPECT_NO_THROW(reg.kill_entity(entity));
}

TEST(RegistryTest, KillEntityRemovesComponents) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{10.0f, 20.0f});
    reg.add_component(entity, Velocity{1.0f, 2.0f});
    
    auto &positions = reg.get_components<Position>();
    auto &velocities = reg.get_components<Velocity>();
    
    EXPECT_TRUE(positions.has(entity.getId()));
    EXPECT_TRUE(velocities.has(entity.getId()));
    
    reg.kill_entity(entity);
    
    EXPECT_FALSE(positions.has(entity.getId()));
    EXPECT_FALSE(velocities.has(entity.getId()));
}

TEST(RegistryTest, ReuseDeadEntityId) {
    Engine::registry reg;
    
    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    
    EXPECT_EQ(e1.getId(), 0);
    EXPECT_EQ(e2.getId(), 1);
    
    reg.kill_entity(e1);
    
    auto e3 = reg.spawn_entity();
    EXPECT_EQ(e3.getId(), 0); // Should reuse ID 0
}

// ============================================================================
// ADD COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, AddComponentRvalue) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    auto &pos = reg.add_component(entity, Position{10.0f, 20.0f});
    
    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 10.0f);
    EXPECT_EQ(pos->y, 20.0f);
}

TEST(RegistryTest, AddComponentLvalue) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    Position pos_value{15.0f, 25.0f};
    auto &pos = reg.add_component(entity, std::move(pos_value));
    
    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 15.0f);
    EXPECT_EQ(pos->y, 25.0f);
}

TEST(RegistryTest, AddMultipleComponentsToEntity) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    reg.register_component<Health>();
    
    auto entity = reg.spawn_entity();
    
    reg.add_component(entity, Position{1.0f, 2.0f});
    reg.add_component(entity, Velocity{3.0f, 4.0f});
    reg.add_component(entity, Health{100});
    
    auto &positions = reg.get_components<Position>();
    auto &velocities = reg.get_components<Velocity>();
    auto &healths = reg.get_components<Health>();
    
    EXPECT_TRUE(positions.has(entity.getId()));
    EXPECT_TRUE(velocities.has(entity.getId()));
    EXPECT_TRUE(healths.has(entity.getId()));
}

TEST(RegistryTest, AddComponentToMultipleEntities) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    auto e3 = reg.spawn_entity();
    
    reg.add_component(e1, Position{1.0f, 1.0f});
    reg.add_component(e2, Position{2.0f, 2.0f});
    reg.add_component(e3, Position{3.0f, 3.0f});
    
    auto &positions = reg.get_components<Position>();
    
    EXPECT_TRUE(positions.has(e1.getId()));
    EXPECT_TRUE(positions.has(e2.getId()));
    EXPECT_TRUE(positions.has(e3.getId()));
    
    EXPECT_EQ(positions[e1.getId()]->x, 1.0f);
    EXPECT_EQ(positions[e2.getId()]->x, 2.0f);
    EXPECT_EQ(positions[e3.getId()]->x, 3.0f);
}

TEST(RegistryTest, OverwriteComponent) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    
    reg.add_component(entity, Position{1.0f, 2.0f});
    auto &positions = reg.get_components<Position>();
    EXPECT_EQ(positions[entity.getId()]->x, 1.0f);
    
    reg.add_component(entity, Position{10.0f, 20.0f});
    EXPECT_EQ(positions[entity.getId()]->x, 10.0f);
}

// ============================================================================
// EMPLACE COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, EmplaceComponent) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    auto &pos = reg.emplace_component<Position>(entity, 5.0f, 10.0f);
    
    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 5.0f);
    EXPECT_EQ(pos->y, 10.0f);
}

TEST(RegistryTest, EmplaceComponentWithSingleArg) {
    Engine::registry reg;
    reg.register_component<Health>();
    
    auto entity = reg.spawn_entity();
    auto &health = reg.emplace_component<Health>(entity, 150);
    
    EXPECT_TRUE(health.has_value());
    EXPECT_EQ(health->hp, 150);
}

TEST(RegistryTest, EmplaceComponentDefaultConstructor) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    auto &pos = reg.emplace_component<Position>(entity);
    
    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 0.0f);
    EXPECT_EQ(pos->y, 0.0f);
}

TEST(RegistryTest, EmplaceComponentString) {
    Engine::registry reg;
    reg.register_component<Name>();
    
    auto entity = reg.spawn_entity();
    auto &name = reg.emplace_component<Name>(entity, "Player");
    
    EXPECT_TRUE(name.has_value());
    EXPECT_EQ(name->value, "Player");
}

// ============================================================================
// REMOVE COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, RemoveComponent) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{1.0f, 2.0f});
    
    auto &positions = reg.get_components<Position>();
    EXPECT_TRUE(positions.has(entity.getId()));
    
    reg.remove_component<Position>(entity);
    
    EXPECT_FALSE(positions.has(entity.getId()));
}

TEST(RegistryTest, RemoveOneOfMultipleComponents) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{1.0f, 2.0f});
    reg.add_component(entity, Velocity{3.0f, 4.0f});
    
    auto &positions = reg.get_components<Position>();
    auto &velocities = reg.get_components<Velocity>();
    
    reg.remove_component<Position>(entity);
    
    EXPECT_FALSE(positions.has(entity.getId()));
    EXPECT_TRUE(velocities.has(entity.getId()));
}

TEST(RegistryTest, RemoveComponentFromMultipleEntities) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    auto e3 = reg.spawn_entity();
    
    reg.add_component(e1, Position{1.0f, 1.0f});
    reg.add_component(e2, Position{2.0f, 2.0f});
    reg.add_component(e3, Position{3.0f, 3.0f});
    
    auto &positions = reg.get_components<Position>();
    
    reg.remove_component<Position>(e2);
    
    EXPECT_TRUE(positions.has(e1.getId()));
    EXPECT_FALSE(positions.has(e2.getId()));
    EXPECT_TRUE(positions.has(e3.getId()));
}

// ============================================================================
// SYSTEMS TESTS
// ============================================================================

TEST(RegistryTest, AddSystemLambda) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    bool system_called = false;
    
    reg.add_system<Engine::sparse_array<Position>>([&system_called](
        Engine::registry &r, Engine::sparse_array<Position> &positions) {
        system_called = true;
    });
    
    reg.run_systems();
    
    EXPECT_TRUE(system_called);
}

TEST(RegistryTest, SystemModifiesComponents) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{0.0f, 0.0f});
    reg.add_component(entity, Velocity{1.0f, 2.0f});
    
    // Movement system
    reg.add_system<Engine::sparse_array<Position>, Engine::sparse_array<Velocity>>([](
        Engine::registry &r, 
        Engine::sparse_array<Position> &positions,
        Engine::sparse_array<Velocity> &velocities) {
        
        for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
            if (positions.has(i) && velocities.has(i)) {
                auto &pos = positions[i];
                auto &vel = velocities[i];
                pos->x += vel->dx;
                pos->y += vel->dy;
            }
        }
    });
    
    auto &positions = reg.get_components<Position>();
    
    reg.run_systems();
    
    EXPECT_EQ(positions[entity.getId()]->x, 1.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 2.0f);
}

TEST(RegistryTest, MultipleSystemsRun) {
    Engine::registry reg;
    reg.register_component<Position>();
    
    int call_count = 0;
    
    reg.add_system<Engine::sparse_array<Position>>([&call_count](
        Engine::registry &r, Engine::sparse_array<Position> &positions) {
        call_count++;
    });
    
    reg.add_system<Engine::sparse_array<Position>>([&call_count](
        Engine::registry &r, Engine::sparse_array<Position> &positions) {
        call_count++;
    });
    
    reg.add_system<Engine::sparse_array<Position>>([&call_count](
        Engine::registry &r, Engine::sparse_array<Position> &positions) {
        call_count++;
    });
    
    reg.run_systems();
    
    EXPECT_EQ(call_count, 3);
}

TEST(RegistryTest, SystemAccessesMultipleComponentTypes) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Health>();
    
    auto entity = reg.spawn_entity();
    reg.add_component(entity, Position{10.0f, 10.0f});
    reg.add_component(entity, Health{50});
    
    bool system_ran = false;
    
    reg.add_system<Engine::sparse_array<Position>, Engine::sparse_array<Health>>([&system_ran](
        Engine::registry &r,
        Engine::sparse_array<Position> &positions,
        Engine::sparse_array<Health> &healths) {
        
        system_ran = true;
        
        for (size_t i = 0; i < positions.size() && i < healths.size(); ++i) {
            if (positions.has(i) && healths.has(i)) {
                EXPECT_TRUE(positions[i].has_value());
                EXPECT_TRUE(healths[i].has_value());
            }
        }
    });
    
    reg.run_systems();
    
    EXPECT_TRUE(system_ran);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST(RegistryTest, CompleteEntityLifecycle) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    reg.register_component<Health>();
    
    // Create entity
    auto entity = reg.spawn_entity();
    
    // Add components
    reg.add_component(entity, Position{0.0f, 0.0f});
    reg.add_component(entity, Velocity{5.0f, 10.0f});
    reg.emplace_component<Health>(entity, 100);
    
    // Verify components exist
    auto &positions = reg.get_components<Position>();
    auto &velocities = reg.get_components<Velocity>();
    auto &healths = reg.get_components<Health>();
    
    EXPECT_TRUE(positions.has(entity.getId()));
    EXPECT_TRUE(velocities.has(entity.getId()));
    EXPECT_TRUE(healths.has(entity.getId()));
    
    // Modify components
    positions[entity.getId()]->x = 100.0f;
    healths[entity.getId()]->hp = 75;
    
    EXPECT_EQ(positions[entity.getId()]->x, 100.0f);
    EXPECT_EQ(healths[entity.getId()]->hp, 75);
    
    // Remove one component
    reg.remove_component<Velocity>(entity);
    EXPECT_FALSE(velocities.has(entity.getId()));
    EXPECT_TRUE(positions.has(entity.getId()));
    EXPECT_TRUE(healths.has(entity.getId()));
    
    // Kill entity
    reg.kill_entity(entity);
    EXPECT_FALSE(positions.has(entity.getId()));
    EXPECT_FALSE(healths.has(entity.getId()));
}

TEST(RegistryTest, MultipleEntitiesWithDifferentComponents) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    reg.register_component<Health>();
    
    auto player = reg.spawn_entity();
    auto enemy = reg.spawn_entity();
    auto projectile = reg.spawn_entity();
    
    // Player has all components
    reg.add_component(player, Position{0.0f, 0.0f});
    reg.add_component(player, Velocity{0.0f, 0.0f});
    reg.add_component(player, Health{100});
    
    // Enemy has position and health
    reg.add_component(enemy, Position{50.0f, 50.0f});
    reg.add_component(enemy, Health{50});
    
    // Projectile has only position and velocity
    reg.add_component(projectile, Position{10.0f, 10.0f});
    reg.add_component(projectile, Velocity{15.0f, 0.0f});
    
    auto &positions = reg.get_components<Position>();
    auto &velocities = reg.get_components<Velocity>();
    auto &healths = reg.get_components<Health>();
    
    // Verify player
    EXPECT_TRUE(positions.has(player.getId()));
    EXPECT_TRUE(velocities.has(player.getId()));
    EXPECT_TRUE(healths.has(player.getId()));
    
    // Verify enemy
    EXPECT_TRUE(positions.has(enemy.getId()));
    EXPECT_FALSE(velocities.has(enemy.getId()));
    EXPECT_TRUE(healths.has(enemy.getId()));
    
    // Verify projectile
    EXPECT_TRUE(positions.has(projectile.getId()));
    EXPECT_TRUE(velocities.has(projectile.getId()));
    EXPECT_FALSE(healths.has(projectile.getId()));
}

TEST(RegistryTest, StressTestManyEntities) {
    Engine::registry reg;
    reg.register_component<Position>();
    reg.register_component<Health>();
    
    const size_t num_entities = 1000;
    std::vector<Engine::entity> entities;
    
    // Spawn many entities
    for (size_t i = 0; i < num_entities; ++i) {
        entities.push_back(reg.spawn_entity());
        reg.add_component(entities.back(), Position{
            static_cast<float>(i), 
            static_cast<float>(i * 2)
        });
        reg.emplace_component<Health>(entities.back(), static_cast<int>(i % 100));
    }
    
    auto &positions = reg.get_components<Position>();
    auto &healths = reg.get_components<Health>();
    
    // Verify all entities have components
    for (size_t i = 0; i < num_entities; ++i) {
        EXPECT_TRUE(positions.has(entities[i].getId()));
        EXPECT_TRUE(healths.has(entities[i].getId()));
    }
    
    // Kill half the entities
    for (size_t i = 0; i < num_entities / 2; ++i) {
        reg.kill_entity(entities[i]);
    }
    
    // Verify first half are dead
    for (size_t i = 0; i < num_entities / 2; ++i) {
        EXPECT_FALSE(positions.has(entities[i].getId()));
        EXPECT_FALSE(healths.has(entities[i].getId()));
    }
    
    // Verify second half are alive
    for (size_t i = num_entities / 2; i < num_entities; ++i) {
        EXPECT_TRUE(positions.has(entities[i].getId()));
        EXPECT_TRUE(healths.has(entities[i].getId()));
    }
}
