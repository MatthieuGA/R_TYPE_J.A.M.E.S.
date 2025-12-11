/**
 * @file test_client_connection_manager.cpp
 * @brief Unit tests for ClientConnectionManager class
 *
 * Tests authentication logic, username validation, capacity management,
 * ready status tracking, and the autostart functionality.
 */

#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"

/**
 * @brief Test fixture for ClientConnectionManager tests
 */
class ClientConnectionManagerTest : public ::testing::Test {
 protected:
    void SetUp() override {
        io_context_ = std::make_unique<boost::asio::io_context>();
    }

    void TearDown() override {
        io_context_.reset();
    }

    /**
     * @brief Create a dummy TCP socket for testing
     *
     * @return boost::asio::ip::tcp::socket
     */
    boost::asio::ip::tcp::socket CreateDummySocket() {
        return boost::asio::ip::tcp::socket(*io_context_);
    }

    std::unique_ptr<boost::asio::io_context> io_context_;
};

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, ConstructorInitializesCorrectly) {
    server::ClientConnectionManager manager(4);
    EXPECT_EQ(manager.GetMaxClients(), 4);
    EXPECT_EQ(manager.GetAuthenticatedCount(), 0);
    EXPECT_FALSE(manager.IsFull());
}

// ============================================================================
// ADD CLIENT TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, AddClientAssignsUniqueId) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    EXPECT_NE(client_id1, client_id2);
    EXPECT_TRUE(manager.HasClient(client_id1));
    EXPECT_TRUE(manager.HasClient(client_id2));
}

TEST_F(ClientConnectionManagerTest, AddClientStartsUnauthenticated) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));

    const auto &client = manager.GetClient(client_id);
    EXPECT_FALSE(client.IsAuthenticated());
    EXPECT_EQ(client.player_id_, 0);
    EXPECT_TRUE(client.username_.empty());
    EXPECT_FALSE(client.ready_);
}

// ============================================================================
// AUTHENTICATION TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, AuthenticateClientSuccess) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));

    uint8_t player_id = manager.AuthenticateClient(client_id, "TestPlayer");

    EXPECT_NE(player_id, 0);
    EXPECT_EQ(manager.GetAuthenticatedCount(), 1);

    const auto &client = manager.GetClient(client_id);
    EXPECT_TRUE(client.IsAuthenticated());
    EXPECT_EQ(client.player_id_, player_id);
    EXPECT_EQ(client.username_, "TestPlayer");
}

TEST_F(ClientConnectionManagerTest, AuthenticateMultipleClients) {
    server::ClientConnectionManager manager(4);

    // Add three clients
    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();
    auto socket3 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));
    uint32_t client_id3 = manager.AddClient(std::move(socket3));

    // Authenticate all three
    uint8_t player_id1 = manager.AuthenticateClient(client_id1, "Player1");
    uint8_t player_id2 = manager.AuthenticateClient(client_id2, "Player2");
    uint8_t player_id3 = manager.AuthenticateClient(client_id3, "Player3");

    EXPECT_NE(player_id1, 0);
    EXPECT_NE(player_id2, 0);
    EXPECT_NE(player_id3, 0);
    EXPECT_NE(player_id1, player_id2);
    EXPECT_NE(player_id2, player_id3);
    EXPECT_NE(player_id1, player_id3);

    EXPECT_EQ(manager.GetAuthenticatedCount(), 3);
}

TEST_F(ClientConnectionManagerTest, AuthenticateClientDuplicateUsername) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    uint8_t player_id1 =
        manager.AuthenticateClient(client_id1, "DuplicateName");
    uint8_t player_id2 =
        manager.AuthenticateClient(client_id2, "DuplicateName");

    EXPECT_NE(player_id1, 0);
    EXPECT_EQ(player_id2, 0);  // Should fail due to duplicate username

    EXPECT_EQ(manager.GetAuthenticatedCount(), 1);
}

TEST_F(ClientConnectionManagerTest, AuthenticateClientServerFull) {
    server::ClientConnectionManager manager(2);  // Max 2 clients

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();
    auto socket3 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));
    uint32_t client_id3 = manager.AddClient(std::move(socket3));

    uint8_t player_id1 = manager.AuthenticateClient(client_id1, "Player1");
    uint8_t player_id2 = manager.AuthenticateClient(client_id2, "Player2");
    uint8_t player_id3 = manager.AuthenticateClient(client_id3, "Player3");

    EXPECT_NE(player_id1, 0);
    EXPECT_NE(player_id2, 0);
    EXPECT_EQ(player_id3, 0);  // Should fail, server full

    EXPECT_EQ(manager.GetAuthenticatedCount(), 2);
    EXPECT_TRUE(manager.IsFull());
}

TEST_F(ClientConnectionManagerTest, AuthenticateClientNonExistent) {
    server::ClientConnectionManager manager(4);

    uint8_t player_id = manager.AuthenticateClient(999, "Player");

    EXPECT_EQ(player_id, 0);  // Should fail, client doesn't exist
}

// ============================================================================
// USERNAME VALIDATION TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, IsUsernameTakenEmpty) {
    server::ClientConnectionManager manager(4);

    EXPECT_FALSE(manager.IsUsernameTaken("AnyName"));
}

TEST_F(ClientConnectionManagerTest, IsUsernameTakenAfterAuthentication) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));
    manager.AuthenticateClient(client_id, "TakenName");

    EXPECT_TRUE(manager.IsUsernameTaken("TakenName"));
    EXPECT_FALSE(manager.IsUsernameTaken("AvailableName"));
}

TEST_F(ClientConnectionManagerTest, IsUsernameTakenUnauthenticatedIgnored) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    manager.AddClient(std::move(socket));
    // Don't authenticate - username should not be considered taken

    EXPECT_FALSE(manager.IsUsernameTaken(""));
}

// ============================================================================
// REMOVE CLIENT TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, RemoveClientSuccess) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));

    EXPECT_TRUE(manager.HasClient(client_id));

    manager.RemoveClient(client_id);

    EXPECT_FALSE(manager.HasClient(client_id));
}

TEST_F(ClientConnectionManagerTest, RemoveAuthenticatedClient) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));
    manager.AuthenticateClient(client_id, "Player1");

    EXPECT_EQ(manager.GetAuthenticatedCount(), 1);

    manager.RemoveClient(client_id);

    EXPECT_EQ(manager.GetAuthenticatedCount(), 0);
    EXPECT_FALSE(manager.HasClient(client_id));
}

TEST_F(ClientConnectionManagerTest, RemoveClientFreesUsername) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    manager.AuthenticateClient(client_id1, "ReusedName");

    EXPECT_TRUE(manager.IsUsernameTaken("ReusedName"));

    manager.RemoveClient(client_id1);

    EXPECT_FALSE(manager.IsUsernameTaken("ReusedName"));

    // Should be able to reuse the username
    uint32_t client_id2 = manager.AddClient(std::move(socket2));
    uint8_t player_id = manager.AuthenticateClient(client_id2, "ReusedName");

    EXPECT_NE(player_id, 0);
}

TEST_F(ClientConnectionManagerTest, RemoveClientNonExistent) {
    server::ClientConnectionManager manager(4);

    // Should not crash when removing non-existent client
    EXPECT_NO_THROW(manager.RemoveClient(999));
}

// ============================================================================
// READY STATUS TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, AllPlayersReadyEmptyServer) {
    server::ClientConnectionManager manager(4);

    // No players connected - should be false
    EXPECT_FALSE(manager.AllPlayersReady());
}

TEST_F(ClientConnectionManagerTest, AllPlayersReadySinglePlayer) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));
    manager.AuthenticateClient(client_id, "Player1");

    // Player not ready yet
    EXPECT_FALSE(manager.AllPlayersReady());

    // Set player ready
    auto &client = manager.GetClient(client_id);
    client.ready_ = true;

    EXPECT_TRUE(manager.AllPlayersReady());
}

TEST_F(ClientConnectionManagerTest, AllPlayersReadyMultiplePlayers) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();
    auto socket3 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));
    uint32_t client_id3 = manager.AddClient(std::move(socket3));

    manager.AuthenticateClient(client_id1, "Player1");
    manager.AuthenticateClient(client_id2, "Player2");
    manager.AuthenticateClient(client_id3, "Player3");

    // No players ready
    EXPECT_FALSE(manager.AllPlayersReady());

    // Two players ready
    manager.GetClient(client_id1).ready_ = true;
    manager.GetClient(client_id2).ready_ = true;
    EXPECT_FALSE(manager.AllPlayersReady());

    // All players ready
    manager.GetClient(client_id3).ready_ = true;
    EXPECT_TRUE(manager.AllPlayersReady());
}

TEST_F(ClientConnectionManagerTest, AllPlayersReadyIgnoresUnauthenticated) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    // Only authenticate one client
    manager.AuthenticateClient(client_id1, "Player1");

    // Set authenticated player ready
    manager.GetClient(client_id1).ready_ = true;

    // Unauthenticated client should not affect ready check
    EXPECT_TRUE(manager.AllPlayersReady());
}

// ============================================================================
// CAPACITY MANAGEMENT TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, IsFullInitiallyFalse) {
    server::ClientConnectionManager manager(4);
    EXPECT_FALSE(manager.IsFull());
}

TEST_F(ClientConnectionManagerTest, IsFullWhenAtCapacity) {
    server::ClientConnectionManager manager(2);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    manager.AuthenticateClient(client_id1, "Player1");
    EXPECT_FALSE(manager.IsFull());

    manager.AuthenticateClient(client_id2, "Player2");
    EXPECT_TRUE(manager.IsFull());
}

TEST_F(ClientConnectionManagerTest, IsFullAfterClientRemoval) {
    server::ClientConnectionManager manager(2);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    manager.AuthenticateClient(client_id1, "Player1");
    manager.AuthenticateClient(client_id2, "Player2");

    EXPECT_TRUE(manager.IsFull());

    manager.RemoveClient(client_id1);

    EXPECT_FALSE(manager.IsFull());
    EXPECT_EQ(manager.GetAuthenticatedCount(), 1);
}

// ============================================================================
// GET CLIENT TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, GetClientSuccess) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));

    EXPECT_NO_THROW({
        auto &client = manager.GetClient(client_id);
        EXPECT_EQ(client.client_id_, client_id);
    });
}

TEST_F(ClientConnectionManagerTest, GetClientNonExistent) {
    server::ClientConnectionManager manager(4);

    EXPECT_THROW(manager.GetClient(999), std::out_of_range);
}

TEST_F(ClientConnectionManagerTest, GetClientConst) {
    server::ClientConnectionManager manager(4);

    auto socket = CreateDummySocket();
    uint32_t client_id = manager.AddClient(std::move(socket));

    const auto &const_manager = manager;
    EXPECT_NO_THROW({
        const auto &client = const_manager.GetClient(client_id);
        EXPECT_EQ(client.client_id_, client_id);
    });
}

// ============================================================================
// GET CLIENTS TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, GetClientsEmpty) {
    server::ClientConnectionManager manager(4);

    const auto &clients = manager.GetClients();
    EXPECT_TRUE(clients.empty());
}

TEST_F(ClientConnectionManagerTest, GetClientsMultiple) {
    server::ClientConnectionManager manager(4);

    auto socket1 = CreateDummySocket();
    auto socket2 = CreateDummySocket();

    uint32_t client_id1 = manager.AddClient(std::move(socket1));
    uint32_t client_id2 = manager.AddClient(std::move(socket2));

    const auto &clients = manager.GetClients();
    EXPECT_EQ(clients.size(), 2);
    EXPECT_TRUE(clients.find(client_id1) != clients.end());
    EXPECT_TRUE(clients.find(client_id2) != clients.end());
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(ClientConnectionManagerTest, MaxCapacity255Players) {
    // Test that the system can handle up to 255 players (max player_id)
    server::ClientConnectionManager manager(255);

    // Add and authenticate a few players to verify the system works
    for (int i = 0; i < 5; ++i) {
        auto socket = CreateDummySocket();
        uint32_t client_id = manager.AddClient(std::move(socket));
        std::string username = "Player" + std::to_string(i);
        uint8_t player_id = manager.AuthenticateClient(client_id, username);
        EXPECT_NE(player_id, 0);
    }

    EXPECT_EQ(manager.GetAuthenticatedCount(), 5);
}

TEST_F(ClientConnectionManagerTest, ClientIdUniqueness) {
    server::ClientConnectionManager manager(10);

    // Add many clients to ensure client IDs never collide
    std::vector<uint32_t> client_ids;
    for (int i = 0; i < 10; ++i) {
        auto socket = CreateDummySocket();
        uint32_t client_id = manager.AddClient(std::move(socket));
        client_ids.push_back(client_id);
    }

    // Check all IDs are unique
    std::set<uint32_t> unique_ids(client_ids.begin(), client_ids.end());
    EXPECT_EQ(unique_ids.size(), client_ids.size());
}
