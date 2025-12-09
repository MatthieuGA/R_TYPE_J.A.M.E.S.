#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "server/Components.hpp"
#include "server/Config.hpp"
#include "server/PacketBuffer.hpp"
#include "server/Packets.hpp"
#include "server/Server.hpp"

// Helper function to create a config for testing
server::Config &getTestConfig() {
    static const char *argv[] = {"test_server"};
    return server::Config::FromCommandLine(1, const_cast<char **>(argv));
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

    EXPECT_NO_THROW(server.Initialize());
}

TEST(ServerTest, GetRegistryAfterInit) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);

    server.Initialize();
    Engine::registry &reg = server.GetRegistry();

    // Registry should be accessible
    EXPECT_NO_THROW({
        auto entity = reg.SpawnEntity();
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

    server.Initialize();
    Engine::registry &reg = server.GetRegistry();

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto entity = reg.SpawnEntity();

    auto &pos =
        reg.AddComponent(entity, server::Component::Position{100.0f, 200.0f});

    EXPECT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 100.0f);
    EXPECT_EQ(pos->y, 200.0f);
}

TEST(ServerTest, SpawnPlayerEntity) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto player = reg.SpawnEntity();

    reg.AddComponent(player, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(player, server::Component::Velocity{0.0f, 0.0f});
    reg.AddComponent(player, server::Component::Health{100, 100});
    reg.AddComponent(player, server::Component::NetworkId{1});
    reg.AddComponent(player, server::Component::Player{1, "TestPlayer"});

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto enemy = reg.SpawnEntity();

    reg.AddComponent(enemy, server::Component::Position{500.0f, 300.0f});
    reg.AddComponent(enemy, server::Component::Velocity{-2.0f, 0.0f});
    reg.AddComponent(enemy, server::Component::Health{50, 50});
    reg.AddComponent(enemy, server::Component::Enemy{10, 100});

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    // Create player
    auto player = reg.SpawnEntity();
    reg.AddComponent(player, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(player, server::Component::Player{1, "Player1"});
    reg.AddComponent(player, server::Component::Health{100, 100});

    // Create enemy
    auto enemy = reg.SpawnEntity();
    reg.AddComponent(enemy, server::Component::Position{100.0f, 100.0f});
    reg.AddComponent(enemy, server::Component::Enemy{15, 50});
    reg.AddComponent(enemy, server::Component::Velocity{-1.0f, 0.0f});

    // Create projectile
    auto projectile = reg.SpawnEntity();
    reg.AddComponent(projectile, server::Component::Position{50.0f, 50.0f});
    reg.AddComponent(projectile, server::Component::Velocity{5.0f, 0.0f});

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto entity = reg.SpawnEntity();

    reg.AddComponent(entity, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(entity, server::Component::Velocity{1.0f, 2.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    EXPECT_EQ(positions[entity.getId()]->x, 0.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 0.0f);

    // Run one update cycle
    server.Update();

    EXPECT_EQ(positions[entity.getId()]->x, 1.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 2.0f);

    // Run another update cycle
    server.Update();

    EXPECT_EQ(positions[entity.getId()]->x, 2.0f);
    EXPECT_EQ(positions[entity.getId()]->y, 4.0f);
}

TEST(ServerTest, MovementSystemMultipleEntities) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    auto e1 = reg.SpawnEntity();
    reg.AddComponent(e1, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(e1, server::Component::Velocity{1.0f, 0.0f});

    auto e2 = reg.SpawnEntity();
    reg.AddComponent(e2, server::Component::Position{10.0f, 10.0f});
    reg.AddComponent(e2, server::Component::Velocity{-2.0f, 3.0f});

    auto e3 = reg.SpawnEntity();
    reg.AddComponent(e3, server::Component::Position{50.0f, 50.0f});
    reg.AddComponent(e3, server::Component::Velocity{0.5f, -1.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.Update();

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    // Entity with position and velocity
    auto moving = reg.SpawnEntity();
    reg.AddComponent(moving, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(moving, server::Component::Velocity{1.0f, 1.0f});

    // Entity with only position (no velocity)
    auto stationary = reg.SpawnEntity();
    reg.AddComponent(stationary, server::Component::Position{10.0f, 10.0f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.Update();

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto entity = reg.SpawnEntity();

    reg.AddComponent(entity, server::Component::Health{75, 100});

    auto &healths = reg.GetComponents<server::Component::Health>();

    EXPECT_EQ(healths[entity.getId()]->current, 75);
    EXPECT_EQ(healths[entity.getId()]->max, 100);
}

TEST(ServerTest, HealthComponentModification) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();
    auto entity = reg.SpawnEntity();

    reg.AddComponent(entity, server::Component::Health{100, 100});

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    auto e1 = reg.SpawnEntity();
    auto e2 = reg.SpawnEntity();
    auto e3 = reg.SpawnEntity();

    reg.AddComponent(e1, server::Component::NetworkId{1001});
    reg.AddComponent(e2, server::Component::NetworkId{1002});
    reg.AddComponent(e3, server::Component::NetworkId{1003});

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    // Spawn player at spawn point
    auto player = reg.SpawnEntity();
    reg.AddComponent(player, server::Component::Position{50.0f, 400.0f});
    reg.AddComponent(player, server::Component::Velocity{0.0f, 0.0f});
    reg.AddComponent(player, server::Component::Health{100, 100});
    reg.AddComponent(player, server::Component::Player{1, "TestPlayer"});

    // Spawn enemy moving towards player
    auto enemy = reg.SpawnEntity();
    reg.AddComponent(enemy, server::Component::Position{800.0f, 400.0f});
    reg.AddComponent(enemy, server::Component::Velocity{-3.0f, 0.0f});
    reg.AddComponent(enemy, server::Component::Health{30, 30});
    reg.AddComponent(enemy, server::Component::Enemy{20, 50});

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Run 10 game ticks
    for (int i = 0; i < 10; i++) {
        server.Update();
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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    // Spawn multiple entities
    std::vector<Engine::entity> entities;
    for (int i = 0; i < 5; i++) {
        auto e = reg.SpawnEntity();
        reg.AddComponent(e,
            server::Component::Position{static_cast<float>(i * 10.0f), 0.0f});
        entities.push_back(e);
    }

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Verify all exist
    for (const auto &e : entities) {
        EXPECT_TRUE(positions.has(e.getId()));
    }

    // Kill some entities
    reg.KillEntity(entities[1]);
    reg.KillEntity(entities[3]);

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    std::vector<Engine::entity> entities;

    for (int i = 0; i < 100; i++) {
        auto e = reg.SpawnEntity();
        reg.AddComponent(e, server::Component::Position{
                                static_cast<float>(i), static_cast<float>(i)});
        reg.AddComponent(e, server::Component::Velocity{1.0f, 1.0f});
        entities.push_back(e);
    }

    auto &positions = reg.GetComponents<server::Component::Position>();

    server.Update();

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
    server.Initialize();

    Engine::registry &reg = server.GetRegistry();

    auto entity = reg.SpawnEntity();
    reg.AddComponent(entity, server::Component::Position{0.0f, 0.0f});
    reg.AddComponent(entity, server::Component::Velocity{0.1f, 0.1f});

    auto &positions = reg.GetComponents<server::Component::Position>();

    // Run many updates
    const int NUM_UPDATES = 1000;
    for (int i = 0; i < NUM_UPDATES; i++) {
        server.Update();
    }

    // Position should be NUM_UPDATES * velocity (with tolerance for FP errors)
    EXPECT_NEAR(positions[entity.getId()]->x, 100.0f, 0.01f);
    EXPECT_NEAR(positions[entity.getId()]->y, 100.0f, 0.01f);
}

// ============================================================================
// TCP PACKET SERIALIZATION AND DESERIALIZATION TESTS
// ============================================================================

TEST(ServerTcpTest, ConnectReqPacketSetUsername) {
    server::network::ConnectReqPacket req;
    req.SetUsername("TestPlayer");

    std::string username = req.GetUsername();
    EXPECT_EQ(username, "TestPlayer");
}

TEST(ServerTcpTest, ConnectReqPacketEmptyUsername) {
    server::network::ConnectReqPacket req;
    req.SetUsername("");

    std::string username = req.GetUsername();
    EXPECT_TRUE(username.empty() ||
                username.find_first_not_of('\0') == std::string::npos);
}

TEST(ServerTcpTest, ConnectReqPacketMaxLengthUsername) {
    server::network::ConnectReqPacket req;

    // 31 characters (max for null-terminated 32-byte array)
    std::string long_name(31, 'X');
    req.SetUsername(long_name);

    std::string retrieved = req.GetUsername();
    EXPECT_EQ(retrieved.size(), 31);
    EXPECT_EQ(retrieved, long_name);
}

TEST(ServerTcpTest, ConnectReqPacketTruncateLongUsername) {
    server::network::ConnectReqPacket req;

    // 40 characters (exceeds 31-char limit)
    std::string too_long(40, 'Y');
    req.SetUsername(too_long);

    std::string retrieved = req.GetUsername();
    // Should be truncated to 31 chars
    EXPECT_EQ(retrieved.size(), 31);
    EXPECT_EQ(retrieved, std::string(31, 'Y'));
}

TEST(ServerTcpTest, ConnectReqPacketSerializeDeserialize) {
    server::network::ConnectReqPacket original;
    original.SetUsername("Alice");

    // Serialize
    server::network::PacketBuffer write_buffer;
    original.Serialize(write_buffer);

    const auto &data = write_buffer.Data();

    // Header (12 bytes) + Payload (32 bytes) = 44 bytes
    EXPECT_EQ(data.size(), 44);

    // Verify opcode
    EXPECT_EQ(data[0],
        static_cast<uint8_t>(server::network::PacketType::ConnectReq));

    // Deserialize payload only
    server::network::PacketBuffer read_buffer(data);
    read_buffer.ReadHeader();  // Skip 12-byte header
    server::network::ConnectReqPacket deserialized =
        server::network::ConnectReqPacket::Deserialize(read_buffer);

    EXPECT_EQ(deserialized.GetUsername(), "Alice");
}

TEST(ServerTcpTest, ConnectAckPacketSerializeOK) {
    server::network::ConnectAckPacket ack;
    ack.player_id = server::network::PlayerId{42};
    ack.status = server::network::ConnectAckPacket::OK;
    ack.reserved = {0, 0};

    server::network::PacketBuffer buffer;
    ack.Serialize(buffer);

    const auto &data = buffer.Data();

    // Header: 12 bytes, Payload: 4 bytes
    EXPECT_EQ(data.size(), 16);

    // Verify opcode (byte 0)
    EXPECT_EQ(data[0],
        static_cast<uint8_t>(server::network::PacketType::ConnectAck));

    // Verify player_id (byte 12)
    EXPECT_EQ(data[12], 42);

    // Verify status (byte 13)
    EXPECT_EQ(data[13], server::network::ConnectAckPacket::OK);
}

TEST(ServerTcpTest, ConnectAckPacketSerializeServerFull) {
    server::network::ConnectAckPacket ack;
    ack.player_id = server::network::PlayerId{0};  // No ID assigned
    ack.status = server::network::ConnectAckPacket::ServerFull;
    ack.reserved = {0, 0};

    server::network::PacketBuffer buffer;
    ack.Serialize(buffer);

    const auto &data = buffer.Data();

    EXPECT_EQ(data.size(), 16);
    EXPECT_EQ(data[12], 0);  // Player ID = 0
    EXPECT_EQ(data[13], server::network::ConnectAckPacket::ServerFull);
}

TEST(ServerTcpTest, ConnectAckPacketSerializeBadUsername) {
    server::network::ConnectAckPacket ack;
    ack.player_id = server::network::PlayerId{0};
    ack.status = server::network::ConnectAckPacket::BadUsername;
    ack.reserved = {0, 0};

    server::network::PacketBuffer buffer;
    ack.Serialize(buffer);

    const auto &data = buffer.Data();

    EXPECT_EQ(data.size(), 16);
    EXPECT_EQ(data[12], 0);
    EXPECT_EQ(data[13], server::network::ConnectAckPacket::BadUsername);
}

TEST(ServerTcpTest, ConnectAckPacketSerializeInGame) {
    server::network::ConnectAckPacket ack;
    ack.player_id = server::network::PlayerId{0};
    ack.status = server::network::ConnectAckPacket::InGame;
    ack.reserved = {0, 0};

    server::network::PacketBuffer buffer;
    ack.Serialize(buffer);

    const auto &data = buffer.Data();

    EXPECT_EQ(data.size(), 16);
    EXPECT_EQ(data[12], 0);
    EXPECT_EQ(data[13], server::network::ConnectAckPacket::InGame);
}

TEST(ServerTcpTest, ConnectAckPacketDeserialize) {
    // Create a packet with known values
    server::network::ConnectAckPacket original;
    original.player_id = server::network::PlayerId{7};
    original.status = server::network::ConnectAckPacket::OK;
    original.reserved = {0, 0};

    // Serialize
    server::network::PacketBuffer write_buffer;
    original.Serialize(write_buffer);

    // Deserialize (skip header, read only payload)
    server::network::PacketBuffer read_buffer(write_buffer.Data());
    read_buffer.ReadHeader();  // Skip 12-byte header
    server::network::ConnectAckPacket deserialized =
        server::network::ConnectAckPacket::Deserialize(read_buffer);

    EXPECT_EQ(deserialized.player_id.value, 7);
    EXPECT_EQ(deserialized.status, server::network::ConnectAckPacket::OK);
}

TEST(ServerTcpTest, ConnectAckPacketRoundTrip) {
    for (uint8_t status_val = 0; status_val <= 3; ++status_val) {
        server::network::ConnectAckPacket original;
        original.player_id =
            server::network::PlayerId{static_cast<uint8_t>(status_val + 1)};
        original.status = status_val;
        original.reserved = {0, 0};

        server::network::PacketBuffer write_buffer;
        original.Serialize(write_buffer);

        server::network::PacketBuffer read_buffer(write_buffer.Data());
        read_buffer.ReadHeader();
        server::network::ConnectAckPacket deserialized =
            server::network::ConnectAckPacket::Deserialize(read_buffer);

        EXPECT_EQ(deserialized.player_id.value, status_val + 1);
        EXPECT_EQ(deserialized.status, status_val);
    }
}

// ============================================================================
// PACKET HEADER TESTS
// ============================================================================

TEST(ServerTcpTest, ConnectReqPacketHeader) {
    server::network::ConnectReqPacket req;
    auto header = req.MakeHeader();

    EXPECT_EQ(header.op_code,
        static_cast<uint8_t>(server::network::PacketType::ConnectReq));
    EXPECT_EQ(header.payload_size, 32);
}

TEST(ServerTcpTest, ConnectAckPacketHeader) {
    server::network::ConnectAckPacket ack;
    auto header = ack.MakeHeader();

    EXPECT_EQ(header.op_code,
        static_cast<uint8_t>(server::network::PacketType::ConnectAck));
    EXPECT_EQ(header.payload_size, 4);
}

// ============================================================================
// USERNAME VALIDATION TESTS (Using Packet Interface)
// ============================================================================

TEST(ServerTcpTest, UsernameNullPadding) {
    server::network::ConnectReqPacket req;
    req.SetUsername("Test");

    // Verify the array is null-padded
    bool has_null = false;
    for (size_t i = 4; i < 32; ++i) {
        if (req.username[i] == '\0') {
            has_null = true;
            break;
        }
    }
    EXPECT_TRUE(has_null);
}

TEST(ServerTcpTest, UsernameSpecialCharacters) {
    server::network::ConnectReqPacket req;
    req.SetUsername("User_123-ABC");

    EXPECT_EQ(req.GetUsername(), "User_123-ABC");
}

TEST(ServerTcpTest, UsernameUnicodeHandling) {
    server::network::ConnectReqPacket req;
    // ASCII-compatible characters
    req.SetUsername("Player1");

    EXPECT_EQ(req.GetUsername(), "Player1");
}

// ============================================================================
// INTEGRATION TEST HELPERS
// ============================================================================

TEST(ServerTcpTest, ServerInitializationWithNetworking) {
    boost::asio::io_context io;
    server::Config &config = getTestConfig();
    server::Server server(config, io);

    EXPECT_NO_THROW(server.Initialize());
}

TEST(ServerTcpTest, MultipleConnectAckStatuses) {
    std::vector<server::network::ConnectAckPacket::Status> statuses = {
        server::network::ConnectAckPacket::OK,
        server::network::ConnectAckPacket::ServerFull,
        server::network::ConnectAckPacket::BadUsername,
        server::network::ConnectAckPacket::InGame};

    for (auto status : statuses) {
        server::network::ConnectAckPacket ack;
        ack.player_id = server::network::PlayerId{1};
        ack.status = status;
        ack.reserved = {0, 0};

        server::network::PacketBuffer buffer;
        EXPECT_NO_THROW(ack.Serialize(buffer));

        const auto &data = buffer.Data();
        EXPECT_EQ(data.size(), 16);
        EXPECT_EQ(data[13], status);
    }
}
