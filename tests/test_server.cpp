#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>

#include "server/Components.hpp"
#include "server/Config.hpp"
#include "server/Server.hpp"

// Helper function to create a config for testing
server::Config &getTestConfig() {
    static const char *argv[] = {"test_server"};
    return server::Config::fromCommandLine(1, const_cast<char **>(argv));
}

// ============================================================================
// SERVER INITIALIZATION TESTS
// ============================================================================

TEST(ServerTest, ServerConstruction) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();

    EXPECT_NO_THROW({ server::Server server(config, io); });
}

TEST(ServerTest, ServerInitialize) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);

    EXPECT_NO_THROW(server.initialize());
}

TEST(ServerTest, GetRegistryAfterInit) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);

    server.initialize();
    Engine::registry &reg = server.getRegistry();

    // Registry should be accessible
    EXPECT_NO_THROW({
        auto entity = reg.spawn_entity();
        EXPECT_EQ(entity.getId(), 0);
    });
}

// ============================================================================
// COMPONENT REGISTRATION TESTS
// ============================================================================

TEST(ServerTest, ComponentsRegisteredAfterInit) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);

    server.initialize();
    Engine::registry &reg = server.getRegistry();

    // All components should be registered and accessible
    EXPECT_NO_THROW({
        auto &positions = reg.GetComponents<server::Component::Position>();
        auto &velocities = reg.GetComponents<server::Component::Velocity>();
        auto &healths = reg.GetComponents<server::Component::Health>();
        auto &network_ids = reg.GetComponents<server::Component::NetworkId>();
        auto &players = reg.GetComponents<server::Component::Player>();
        auto &enemies = reg.GetComponents<server::Component::Enemy>();
    });
}

// ============================================================================
// ENTITY AND COMPONENT TESTS
// ============================================================================

TEST(ServerTest, SpawnEntityWithPosition) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto entity = reg.spawn_entity();

    auto &pos =
        reg.add_component(entity, server::Component::Position{100.0f, 200.0f});

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 100.0f);
    EXPECT_EQ(pos->y, 200.0f);
}

TEST(ServerTest, SpawnPlayerEntity) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto player = reg.spawn_entity();

    reg.add_component(player, server::Component::Position{0.0f, 0.0f});
    reg.add_component(player, server::Component::Velocity{0.0f, 0.0f});
    reg.add_component(player, server::Component::Health{100, 100});
    reg.add_component(player, server::Component::NetworkId{1});
    reg.add_component(player, server::Component::Player{1, "TestPlayer"});

    auto &positions = reg.GetComponents<server::Component::Position>();
    auto &players = reg.GetComponents<server::Component::Player>();

    EXPECT_TRUE(positions.has(player.getId()));
    EXPECT_TRUE(players.has(player.getId()));
    EXPECT_EQ(players[player.getId()]->player_id, 1);
    EXPECT_EQ(players[player.getId()]->name, "TestPlayer");
}

TEST(ServerTest, SpawnEnemyEntity) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto enemy = reg.spawn_entity();

    reg.add_component(enemy, server::Component::Position{500.0f, 300.0f});
    reg.add_component(enemy, server::Component::Velocity{-2.0f, 0.0f});
    reg.add_component(enemy, server::Component::Health{50, 50});
    reg.add_component(enemy, server::Component::Enemy{10, 100});

    auto &positions = reg.GetComponents<server::Component::Position>();
    auto &enemies = reg.GetComponents<server::Component::Enemy>();

    EXPECT_TRUE(positions.has(enemy.getId()));
    EXPECT_TRUE(enemies.has(enemy.getId()));
    EXPECT_EQ(enemies[enemy.getId()]->damage, 10);
    EXPECT_EQ(enemies[enemy.getId()]->points, 100);
}

TEST(ServerTest, MultipleEntitiesWithDifferentComponents) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    // Create player
    auto player = reg.spawn_entity();
    reg.add_component(player, server::Component::Position{0.0f, 0.0f});
    reg.add_component(player, server::Component::Player{1, "Player1"});
    reg.add_component(player, server::Component::Health{100, 100});

    // Create enemy
    auto enemy = reg.spawn_entity();
    reg.add_component(enemy, server::Component::Position{100.0f, 100.0f});
    reg.add_component(enemy, server::Component::Enemy{15, 50});
    reg.add_component(enemy, server::Component::Velocity{-1.0f, 0.0f});

    // Create projectile
    auto projectile = reg.spawn_entity();
    reg.add_component(projectile, server::Component::Position{50.0f, 50.0f});
    reg.add_component(projectile, server::Component::Velocity{5.0f, 0.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();
    auto &players = reg.GetComponents<server::Component::Player>();
    auto &enemies = reg.GetComponents<server::Component::Enemy>();

    EXPECT_TRUE(positions.has(player.getId()));
    EXPECT_TRUE(players.has(player.getId()));

    EXPECT_TRUE(positions.has(enemy.getId()));
    EXPECT_TRUE(enemies.has(enemy.getId()));

    EXPECT_TRUE(positions.has(projectile.getId()));
    EXPECT_FALSE(players.has(projectile.getId()));
    EXPECT_FALSE(enemies.has(projectile.getId()));
}

// ============================================================================
// MOVEMENT SYSTEM TESTS
// ============================================================================

TEST(ServerTest, MovementSystemUpdatesPosition) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto entity = reg.spawn_entity();

    reg.add_component(entity, server::Component::Position{0.0f, 0.0f});
    reg.add_component(entity, server::Component::Velocity{1.0f, 2.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    EXPECT_EQ(positions[entity.getId()]->x, 0.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 0.0f);

    // Run one update cycle
    server.update();

    EXPECT_EQ(positions[entity.getId()]->x, 1.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 2.0f);

    // Run another update cycle
    server.update();

    EXPECT_EQ(positions[entity.getId()]->x, 2.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 4.0f);
}

TEST(ServerTest, MovementSystemMultipleEntities) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    auto e1 = reg.spawn_entity();
    reg.add_component(e1, server::Component::Position{0.0f, 0.0f});
    reg.add_component(e1, server::Component::Velocity{1.0f, 0.0f});

    auto e2 = reg.spawn_entity();
    reg.add_component(e2, server::Component::Position{10.0f, 10.0f});
    reg.add_component(e2, server::Component::Velocity{-2.0f, 3.0f});

    auto e3 = reg.spawn_entity();
    reg.add_component(e3, server::Component::Position{50.0f, 50.0f});
    reg.add_component(e3, server::Component::Velocity{0.5f, -1.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.update();

    EXPECT_EQ(positions[e1.getId()]->x, 1.0f);
    EXPECT_EQ(positions[e1.getId()]->y, 0.0f);

    EXPECT_EQ(positions[e2.getId()]->x, 8.0f);
    EXPECT_EQ(positions[e2.getId()]->y, 13.0f);

    EXPECT_EQ(positions[e3.getId()]->x, 50.5f);
    EXPECT_EQ(positions[e3.getId()]->y, 49.0f);
}

TEST(ServerTest, MovementSystemIgnoresEntitiesWithoutVelocity) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    // Entity with position and velocity
    auto moving = reg.spawn_entity();
    reg.add_component(moving, server::Component::Position{0.0f, 0.0f});
    reg.add_component(moving, server::Component::Velocity{1.0f, 1.0f});

    // Entity with only position (no velocity)
    auto stationary = reg.spawn_entity();
    reg.add_component(stationary, server::Component::Position{10.0f, 10.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.update();

    // Moving entity should have moved
    EXPECT_EQ(positions[moving.getId()]->x, 1.0f);
    EXPECT_EQ(positions[moving.getId()]->y, 1.0f);

    // Stationary entity should not have moved
    EXPECT_EQ(positions[stationary.getId()]->x, 10.0f);
    EXPECT_EQ(positions[stationary.getId()]->y, 10.0f);
}

// ============================================================================
// HEALTH COMPONENT TESTS
// ============================================================================

TEST(ServerTest, HealthComponentInitialization) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto entity = reg.spawn_entity();

    reg.add_component(entity, server::Component::Health{75, 100});

    auto &healths = reg.GetComponents<server::Component::Health>();

    EXPECT_EQ(healths[entity.getId()]->current, 75);
    EXPECT_EQ(healths[entity.getId()]->max, 100);
}

TEST(ServerTest, HealthComponentModification) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();
    auto entity = reg.spawn_entity();

    reg.add_component(entity, server::Component::Health{100, 100});

    auto &healths = reg.GetComponents<server::Component::Health>();

    // Take damage
    healths[entity.getId()]->current -= 25;
    EXPECT_EQ(healths[entity.getId()]->current, 75);

    // Heal
    healths[entity.getId()]->current += 10;
    EXPECT_EQ(healths[entity.getId()]->current, 85);

    // Don't overheal
    healths[entity.getId()]->current = healths[entity.getId()]->max;
    EXPECT_EQ(healths[entity.getId()]->current, 100);
}

// ============================================================================
// NETWORK ID TESTS
// ============================================================================

TEST(ServerTest, NetworkIdComponent) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    auto e3 = reg.spawn_entity();

    reg.add_component(e1, server::Component::NetworkId{1001});
    reg.add_component(e2, server::Component::NetworkId{1002});
    reg.add_component(e3, server::Component::NetworkId{1003});

    auto &network_ids = reg.GetComponents<server::Component::NetworkId>();

    EXPECT_EQ(network_ids[e1.getId()]->id, 1001);
    EXPECT_EQ(network_ids[e2.getId()]->id, 1002);
    EXPECT_EQ(network_ids[e3.getId()]->id, 1003);
}

// ============================================================================
// GAME SCENARIO TESTS
// ============================================================================

TEST(ServerTest, SimpleGameScenario) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    // Spawn player at spawn point
    auto player = reg.spawn_entity();
    reg.add_component(player, server::Component::Position{50.0f, 400.0f});
    reg.add_component(player, server::Component::Velocity{0.0f, 0.0f});
    reg.add_component(player, server::Component::Health{100, 100});
    reg.add_component(player, server::Component::Player{1, "TestPlayer"});

    // Spawn enemy moving towards player
    auto enemy = reg.spawn_entity();
    reg.add_component(enemy, server::Component::Position{800.0f, 400.0f});
    reg.add_component(enemy, server::Component::Velocity{-3.0f, 0.0f});
    reg.add_component(enemy, server::Component::Health{30, 30});
    reg.add_component(enemy, server::Component::Enemy{20, 50});

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Run 10 game ticks
    for (int i = 0; i < 10; i++) {
        server.update();
    }

    // Enemy should have moved 30 pixels left
    EXPECT_EQ(positions[enemy.getId()]->x, 770.0f);
    EXPECT_EQ(positions[enemy.getId()]->y, 400.0f);

    // Player should not have moved (velocity is 0)
    EXPECT_EQ(positions[player.getId()]->x, 50.0f);
    EXPECT_EQ(positions[player.getId()]->y, 400.0f);
}

TEST(ServerTest, EntityCleanup) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    // Spawn multiple entities
    std::vector<Engine::entity> entities;
    for (int i = 0; i < 5; i++) {
        auto e = reg.spawn_entity();
        reg.add_component(e,
            server::Component::Position{static_cast<float>(i * 10.0f), 0.0f});
        entities.push_back(e);
    }

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Verify all exist
    for (const auto &e : entities) {
        EXPECT_TRUE(positions.has(e.getId()));
    }

    // Kill some entities
    reg.kill_entity(entities[1]);
    reg.kill_entity(entities[3]);

    // Verify correct ones are gone
    EXPECT_TRUE(positions.has(entities[0].getId()));
    EXPECT_FALSE(positions.has(entities[1].getId()));
    EXPECT_TRUE(positions.has(entities[2].getId()));
    EXPECT_FALSE(positions.has(entities[3].getId()));
    EXPECT_TRUE(positions.has(entities[4].getId()));
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST(ServerTest, StressManyEntities) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    std::vector<Engine::entity> entities;

    for (int i = 0; i < 100; i++) {
        auto e = reg.spawn_entity();
        reg.add_component(e, server::Component::Position{static_cast<float>(i),
                                 static_cast<float>(i)});
        reg.add_component(e, server::Component::Velocity{1.0f, 1.0f});
        entities.push_back(e);
    }

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.update();

    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(
            positions[entities[i].getId()]->x, static_cast<float>(i + 1));
        EXPECT_EQ(
            positions[entities[i].getId()]->y, static_cast<float>(i + 1));
    }
}

TEST(ServerTest, StressMultipleUpdates) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.initialize();

    Engine::registry &reg = server.getRegistry();

    auto entity = reg.spawn_entity();
    reg.add_component(entity, server::Component::Position{0.0f, 0.0f});
    reg.add_component(entity, server::Component::Velocity{0.1f, 0.1f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Run many updates
    const int NUM_UPDATES = 1000;
    for (int i = 0; i < NUM_UPDATES; i++) {
        server.update();
    }

    // Position should be NUM_UPDATES * velocity (with tolerance for FP errors)
    EXPECT_NEAR(positions[entity.getId()]->x, 100.0f, 0.01f);
    EXPECT_NEAR(positions[entity.getId()]->y, 100.0f, 0.01f);
}
