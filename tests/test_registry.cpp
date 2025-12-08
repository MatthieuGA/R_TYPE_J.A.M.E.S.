#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "include/entity.hpp"
#include "include/registry.hpp"

// Test components
struct Position {
    float x;
    float y;

    explicit Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Velocity {
    float dx;
    float dy;

    explicit Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
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

    auto &positions = reg.RegisterComponent<Position>();

    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, RegisterMultipleComponents) {
    Engine::registry reg;

    auto &positions = reg.RegisterComponent<Position>();
    auto &velocities = reg.RegisterComponent<Velocity>();
    auto &healths = reg.RegisterComponent<Health>();

    EXPECT_EQ(positions.size(), 0);
    EXPECT_EQ(velocities.size(), 0);
    EXPECT_EQ(healths.size(), 0);
}

TEST(RegistryTest, RegisterComponentTwiceThrows) {
    Engine::registry reg;

    reg.RegisterComponent<Position>();

    EXPECT_THROW(reg.RegisterComponent<Position>(), std::runtime_error);
}

TEST(RegistryTest, GetComponents) {
    Engine::registry reg;

    reg.RegisterComponent<Position>();
    auto &positions = reg.GetComponents<Position>();

    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, GetComponentsConst) {
    Engine::registry reg;

    reg.RegisterComponent<Position>();
    const auto &const_reg = reg;
    const auto &positions = const_reg.GetComponents<Position>();

    EXPECT_EQ(positions.size(), 0);
}

TEST(RegistryTest, GetComponentsNotRegisteredThrows) {
    Engine::registry reg;

    EXPECT_THROW(reg.GetComponents<Position>(), std::runtime_error);
}

// ============================================================================
// ENTITY MANAGEMENT TESTS
// ============================================================================

TEST(RegistryTest, SpawnEntity) {
    Engine::registry reg;

    auto entity = reg.SpawnEntity();

    EXPECT_EQ(entity.GetId(), 0);
}

TEST(RegistryTest, SpawnMultipleEntities) {
    Engine::registry reg;

    auto e1 = reg.SpawnEntity();
    auto e2 = reg.SpawnEntity();
    auto e3 = reg.SpawnEntity();

    EXPECT_EQ(e1.GetId(), 0);
    EXPECT_EQ(e2.GetId(), 1);
    EXPECT_EQ(e3.GetId(), 2);
}

TEST(RegistryTest, EntityFromIndex) {
    Engine::registry reg;

    auto entity = reg.EntityFromIndex(42);

    EXPECT_EQ(entity.GetId(), 42);
}

TEST(RegistryTest, KillEntity) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{10.0f, 20.0f});

    EXPECT_NO_THROW(reg.KillEntity(entity));
}

TEST(RegistryTest, KillEntityRemovesComponents) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{10.0f, 20.0f});
    reg.AddComponent(entity, Velocity{1.0f, 2.0f});

    auto &positions = reg.GetComponents<Position>();
    auto &velocities = reg.GetComponents<Velocity>();

    EXPECT_TRUE(positions.has(entity.GetId()));
    EXPECT_TRUE(velocities.has(entity.GetId()));

    reg.KillEntity(entity);

    EXPECT_FALSE(positions.has(entity.GetId()));
    EXPECT_FALSE(velocities.has(entity.GetId()));
}

TEST(RegistryTest, ReuseDeadEntityId) {
    Engine::registry reg;

    auto e1 = reg.SpawnEntity();
    auto e2 = reg.SpawnEntity();

    EXPECT_EQ(e1.GetId(), 0);
    EXPECT_EQ(e2.GetId(), 1);

    reg.KillEntity(e1);

    auto e3 = reg.SpawnEntity();
    EXPECT_EQ(e3.GetId(), 0);  // Should reuse ID 0
}

// ============================================================================
// ADD COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, AddComponentRvalue) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    auto &pos = reg.AddComponent(entity, Position{10.0f, 20.0f});

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 10.0f);
    EXPECT_EQ(pos->y, 20.0f);
}

TEST(RegistryTest, AddComponentLvalue) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    Position pos_value{15.0f, 25.0f};
    auto &pos = reg.AddComponent(entity, std::move(pos_value));

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 15.0f);
    EXPECT_EQ(pos->y, 25.0f);
}

TEST(RegistryTest, AddMultipleComponentsToEntity) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();
    reg.RegisterComponent<Health>();

    auto entity = reg.SpawnEntity();

    reg.AddComponent(entity, Position{1.0f, 2.0f});
    reg.AddComponent(entity, Velocity{3.0f, 4.0f});
    reg.AddComponent(entity, Health{100});

    auto &positions = reg.GetComponents<Position>();
    auto &velocities = reg.GetComponents<Velocity>();
    auto &healths = reg.GetComponents<Health>();

    EXPECT_TRUE(positions.has(entity.GetId()));
    EXPECT_TRUE(velocities.has(entity.GetId()));
    EXPECT_TRUE(healths.has(entity.GetId()));
}

TEST(RegistryTest, AddComponentToMultipleEntities) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto e1 = reg.SpawnEntity();
    auto e2 = reg.SpawnEntity();
    auto e3 = reg.SpawnEntity();

    reg.AddComponent(e1, Position{1.0f, 1.0f});
    reg.AddComponent(e2, Position{2.0f, 2.0f});
    reg.AddComponent(e3, Position{3.0f, 3.0f});

    auto &positions = reg.GetComponents<Position>();

    EXPECT_TRUE(positions.has(e1.GetId()));
    EXPECT_TRUE(positions.has(e2.GetId()));
    EXPECT_TRUE(positions.has(e3.GetId()));

    EXPECT_EQ(positions[e1.GetId()]->x, 1.0f);
    EXPECT_EQ(positions[e2.GetId()]->x, 2.0f);
    EXPECT_EQ(positions[e3.GetId()]->x, 3.0f);
}

TEST(RegistryTest, OverwriteComponent) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();

    reg.AddComponent(entity, Position{1.0f, 2.0f});
    auto &positions = reg.GetComponents<Position>();
    EXPECT_EQ(positions[entity.GetId()]->x, 1.0f);

    reg.AddComponent(entity, Position{10.0f, 20.0f});
    EXPECT_EQ(positions[entity.GetId()]->x, 10.0f);
}

// ============================================================================
// EMPLACE COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, EmplaceComponent) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    auto &pos = reg.EmplaceComponent<Position>(entity, 5.0f, 10.0f);

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 5.0f);
    EXPECT_EQ(pos->y, 10.0f);
}

TEST(RegistryTest, EmplaceComponentWithSingleArg) {
    Engine::registry reg;
    reg.RegisterComponent<Health>();

    auto entity = reg.SpawnEntity();
    auto &health = reg.EmplaceComponent<Health>(entity, 150);

    EXPECT_TRUE(health.has_value());
    EXPECT_EQ(health->hp, 150);
}

TEST(RegistryTest, EmplaceComponentDefaultConstructor) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    auto &pos = reg.EmplaceComponent<Position>(entity);

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 0.0f);
    EXPECT_EQ(pos->y, 0.0f);
}

TEST(RegistryTest, EmplaceComponentString) {
    Engine::registry reg;
    reg.RegisterComponent<Name>();

    auto entity = reg.SpawnEntity();
    auto &name = reg.EmplaceComponent<Name>(entity, "Player");

    EXPECT_TRUE(name.has_value());
    EXPECT_EQ(name->value, "Player");
}

// ============================================================================
// REMOVE COMPONENT TESTS
// ============================================================================

TEST(RegistryTest, RemoveComponent) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{1.0f, 2.0f});

    auto &positions = reg.GetComponents<Position>();
    EXPECT_TRUE(positions.has(entity.GetId()));

    reg.RemoveComponent<Position>(entity);

    EXPECT_FALSE(positions.has(entity.GetId()));
}

TEST(RegistryTest, RemoveOneOfMultipleComponents) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{1.0f, 2.0f});
    reg.AddComponent(entity, Velocity{3.0f, 4.0f});

    auto &positions = reg.GetComponents<Position>();
    auto &velocities = reg.GetComponents<Velocity>();

    reg.RemoveComponent<Position>(entity);

    EXPECT_FALSE(positions.has(entity.GetId()));
    EXPECT_TRUE(velocities.has(entity.GetId()));
}

TEST(RegistryTest, RemoveComponentFromMultipleEntities) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    auto e1 = reg.SpawnEntity();
    auto e2 = reg.SpawnEntity();
    auto e3 = reg.SpawnEntity();

    reg.AddComponent(e1, Position{1.0f, 1.0f});
    reg.AddComponent(e2, Position{2.0f, 2.0f});
    reg.AddComponent(e3, Position{3.0f, 3.0f});

    auto &positions = reg.GetComponents<Position>();

    reg.RemoveComponent<Position>(e2);

    EXPECT_TRUE(positions.has(e1.GetId()));
    EXPECT_FALSE(positions.has(e2.GetId()));
    EXPECT_TRUE(positions.has(e3.GetId()));
}

// ============================================================================
// SYSTEMS TESTS
// ============================================================================

TEST(RegistryTest, AddSystemLambda) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    bool system_called = false;

    reg.AddSystem<Engine::sparse_array<Position>>(
        [&system_called](
            Engine::registry &r, Engine::sparse_array<Position> &positions) {
            system_called = true;
        });

    reg.RunSystems();

    EXPECT_TRUE(system_called);
}

TEST(RegistryTest, SystemModifiesComponents) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{0.0f, 0.0f});
    reg.AddComponent(entity, Velocity{1.0f, 2.0f});

    // Movement system
    reg.AddSystem<Engine::sparse_array<Position>,
        Engine::sparse_array<Velocity>>(
        [](Engine::registry &r, Engine::sparse_array<Position> &positions,
            Engine::sparse_array<Velocity> &velocities) {
            for (size_t i = 0; i < positions.size() && i < velocities.size();
                ++i) {
                if (positions.has(i) && velocities.has(i)) {
                    auto &pos = positions[i];
                    auto &vel = velocities[i];
                    pos->x += vel->dx;
                    pos->y += vel->dy;
                }
            }
        });

    auto &positions = reg.GetComponents<Position>();

    reg.RunSystems();

    EXPECT_EQ(positions[entity.GetId()]->x, 1.0f);
    EXPECT_EQ(positions[entity.GetId()]->y, 2.0f);
}

TEST(RegistryTest, MultipleSystemsRun) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();

    int call_count = 0;

    reg.AddSystem<Engine::sparse_array<Position>>(
        [&call_count](Engine::registry &r,
            Engine::sparse_array<Position> &positions) { call_count++; });

    reg.AddSystem<Engine::sparse_array<Position>>(
        [&call_count](Engine::registry &r,
            Engine::sparse_array<Position> &positions) { call_count++; });

    reg.AddSystem<Engine::sparse_array<Position>>(
        [&call_count](Engine::registry &r,
            Engine::sparse_array<Position> &positions) { call_count++; });

    reg.RunSystems();

    EXPECT_EQ(call_count, 3);
}

TEST(RegistryTest, SystemAccessesMultipleComponentTypes) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Health>();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, Position{10.0f, 10.0f});
    reg.AddComponent(entity, Health{50});

    bool system_ran = false;

    reg.AddSystem<Engine::sparse_array<Position>,
        Engine::sparse_array<Health>>(
        [&system_ran](Engine::registry &r,
            Engine::sparse_array<Position> &positions,
            Engine::sparse_array<Health> &healths) {
            system_ran = true;

            for (size_t i = 0; i < positions.size() && i < healths.size();
                ++i) {
                if (positions.has(i) && healths.has(i)) {
                    EXPECT_TRUE(positions[i].has_value());
                    EXPECT_TRUE(healths[i].has_value());
                }
            }
        });

    reg.RunSystems();

    EXPECT_TRUE(system_ran);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST(RegistryTest, CompleteEntityLifecycle) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();
    reg.RegisterComponent<Health>();

    // Create entity
    auto entity = reg.SpawnEntity();

    // Add components
    reg.AddComponent(entity, Position{0.0f, 0.0f});
    reg.AddComponent(entity, Velocity{5.0f, 10.0f});
    reg.EmplaceComponent<Health>(entity, 100);

    // Verify components exist
    auto &positions = reg.GetComponents<Position>();
    auto &velocities = reg.GetComponents<Velocity>();
    auto &healths = reg.GetComponents<Health>();

    EXPECT_TRUE(positions.has(entity.GetId()));
    EXPECT_TRUE(velocities.has(entity.GetId()));
    EXPECT_TRUE(healths.has(entity.GetId()));

    // Modify components
    positions[entity.GetId()]->x = 100.0f;
    healths[entity.GetId()]->hp = 75;

    EXPECT_EQ(positions[entity.GetId()]->x, 100.0f);
    EXPECT_EQ(healths[entity.GetId()]->hp, 75);

    // Remove one component
    reg.RemoveComponent<Velocity>(entity);
    EXPECT_FALSE(velocities.has(entity.GetId()));
    EXPECT_TRUE(positions.has(entity.GetId()));
    EXPECT_TRUE(healths.has(entity.GetId()));

    // Kill entity
    reg.KillEntity(entity);
    EXPECT_FALSE(positions.has(entity.GetId()));
    EXPECT_FALSE(healths.has(entity.GetId()));
}

TEST(RegistryTest, MultipleEntitiesWithDifferentComponents) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Velocity>();
    reg.RegisterComponent<Health>();

    auto player = reg.SpawnEntity();
    auto enemy = reg.SpawnEntity();
    auto projectile = reg.SpawnEntity();

    // Player has all components
    reg.AddComponent(player, Position{0.0f, 0.0f});
    reg.AddComponent(player, Velocity{0.0f, 0.0f});
    reg.AddComponent(player, Health{100});

    // Enemy has position and health
    reg.AddComponent(enemy, Position{50.0f, 50.0f});
    reg.AddComponent(enemy, Health{50});

    // Projectile has only position and velocity
    reg.AddComponent(projectile, Position{10.0f, 10.0f});
    reg.AddComponent(projectile, Velocity{15.0f, 0.0f});

    auto &positions = reg.GetComponents<Position>();
    auto &velocities = reg.GetComponents<Velocity>();
    auto &healths = reg.GetComponents<Health>();

    // Verify player
    EXPECT_TRUE(positions.has(player.GetId()));
    EXPECT_TRUE(velocities.has(player.GetId()));
    EXPECT_TRUE(healths.has(player.GetId()));

    // Verify enemy
    EXPECT_TRUE(positions.has(enemy.GetId()));
    EXPECT_FALSE(velocities.has(enemy.GetId()));
    EXPECT_TRUE(healths.has(enemy.GetId()));

    // Verify projectile
    EXPECT_TRUE(positions.has(projectile.GetId()));
    EXPECT_TRUE(velocities.has(projectile.GetId()));
    EXPECT_FALSE(healths.has(projectile.GetId()));
}

TEST(RegistryTest, StressTestManyEntities) {
    Engine::registry reg;
    reg.RegisterComponent<Position>();
    reg.RegisterComponent<Health>();

    const size_t num_entities = 1000;
    std::vector<Engine::entity> entities;

    // Spawn many entities
    for (size_t i = 0; i < num_entities; ++i) {
        entities.push_back(reg.SpawnEntity());
        reg.AddComponent(entities.back(),
            Position{static_cast<float>(i), static_cast<float>(i * 2)});
        reg.EmplaceComponent<Health>(
            entities.back(), static_cast<int>(i % 100));
    }

    auto &positions = reg.GetComponents<Position>();
    auto &healths = reg.GetComponents<Health>();

    // Verify all entities have components
    for (size_t i = 0; i < num_entities; ++i) {
        EXPECT_TRUE(positions.has(entities[i].GetId()));
        EXPECT_TRUE(healths.has(entities[i].GetId()));
    }

    // Kill half the entities
    for (size_t i = 0; i < num_entities / 2; ++i) {
        reg.KillEntity(entities[i]);
    }

    // Verify first half are dead
    for (size_t i = 0; i < num_entities / 2; ++i) {
        EXPECT_FALSE(positions.has(entities[i].GetId()));
        EXPECT_FALSE(healths.has(entities[i].GetId()));
    }

    // Verify second half are alive
    for (size_t i = num_entities / 2; i < num_entities; ++i) {
        EXPECT_TRUE(positions.has(entities[i].GetId()));
        EXPECT_TRUE(healths.has(entities[i].GetId()));
    }
}
