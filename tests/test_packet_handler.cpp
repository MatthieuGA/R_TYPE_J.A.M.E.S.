/**
 * @file test_packet_handler.cpp
 * @brief Unit tests for PacketHandler class
 *
 * Tests packet handling logic including authentication (CONNECT_REQ),
 * ready status management (READY_STATUS), disconnection handling,
 * and the autostart functionality when all players are ready.
 */

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"
#include "server/PacketHandler.hpp"
#include "server/PacketSender.hpp"
#include "server/Packets.hpp"

/**
 * @brief Test fixture for PacketHandler tests
 */
class PacketHandlerTest : public ::testing::Test {
 protected:
    void SetUp() override {
        io_context_ = std::make_unique<boost::asio::io_context>();

        // Parse config with test ports
        const char *argv[] = {"test_server", "50102", "50103"};
        config_.emplace(server::Config::Parse(3, const_cast<char **>(argv)));

        network_ = std::make_unique<server::Network>(*config_, *io_context_);
        connection_manager_ =
            std::make_unique<server::ClientConnectionManager>(
                config_->GetMaxPlayers());
        packet_sender_ = std::make_unique<server::PacketSender>(
            *connection_manager_, *network_);
        packet_handler_ = std::make_unique<server::PacketHandler>(
            *connection_manager_, *packet_sender_, *network_);

        // Register handlers
        packet_handler_->RegisterHandlers();
    }

    void TearDown() override {
        packet_handler_.reset();
        packet_sender_.reset();
        connection_manager_.reset();
        io_context_.reset();
    }

    /**
     * @brief Create a dummy TCP socket for testing
     */
    boost::asio::ip::tcp::socket CreateDummySocket() {
        return boost::asio::ip::tcp::socket(*io_context_);
    }

    /**
     * @brief Add an unauthenticated client to the manager
     */
    uint32_t AddTestClient() {
        auto socket = CreateDummySocket();
        return connection_manager_->AddClient(std::move(socket));
    }

    /**
     * @brief Add and authenticate a client
     */
    uint32_t AddAuthenticatedClient(const std::string &username) {
        uint32_t client_id = AddTestClient();
        connection_manager_->AuthenticateClient(client_id, username);
        return client_id;
    }

    std::unique_ptr<boost::asio::io_context> io_context_;
    std::optional<server::Config> config_;
    std::unique_ptr<server::Network> network_;
    std::unique_ptr<server::ClientConnectionManager> connection_manager_;
    std::unique_ptr<server::PacketSender> packet_sender_;
    std::unique_ptr<server::PacketHandler> packet_handler_;
};

// ============================================================================
// HANDLER REGISTRATION TESTS
// ============================================================================

TEST_F(PacketHandlerTest, RegisterHandlersSucceeds) {
    // Should not throw when registering handlers
    EXPECT_NO_THROW(packet_handler_->RegisterHandlers());
}

// ============================================================================
// CONNECT_REQ HANDLING TESTS
// ============================================================================

TEST_F(PacketHandlerTest, HandleConnectReqValidUsername) {
    uint32_t client_id = AddTestClient();

    // Create CONNECT_REQ packet
    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("ValidPlayer");

    // Create packet parse result
    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    // Dispatch packet
    packet_handler_->Dispatch(client_id, result);

    // Verify authentication succeeded
    auto &client = connection_manager_->GetClient(client_id);
    EXPECT_TRUE(client.IsAuthenticated());
    EXPECT_NE(client.player_id_, 0);
    EXPECT_EQ(client.username_, "ValidPlayer");
}

TEST_F(PacketHandlerTest, HandleConnectReqEmptyUsername) {
    uint32_t client_id = AddTestClient();

    // Create CONNECT_REQ packet with empty username
    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("");

    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    packet_handler_->Dispatch(client_id, result);

    // Verify authentication failed
    auto &client = connection_manager_->GetClient(client_id);
    EXPECT_FALSE(client.IsAuthenticated());
    EXPECT_EQ(client.player_id_, 0);
}

TEST_F(PacketHandlerTest, HandleConnectReqDuplicateUsername) {
    // Authenticate first client
    uint32_t client_id1 = AddAuthenticatedClient("DuplicateName");

    // Try to authenticate second client with same username
    uint32_t client_id2 = AddTestClient();

    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("DuplicateName");

    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    packet_handler_->Dispatch(client_id2, result);

    // Verify authentication failed
    auto &client2 = connection_manager_->GetClient(client_id2);
    EXPECT_FALSE(client2.IsAuthenticated());
    EXPECT_EQ(client2.player_id_, 0);
}

TEST_F(PacketHandlerTest, HandleConnectReqServerFull) {
    // Create manager with max 2 clients
    connection_manager_ = std::make_unique<server::ClientConnectionManager>(2);
    packet_sender_ = std::make_unique<server::PacketSender>(
        *connection_manager_, *network_);
    packet_handler_ = std::make_unique<server::PacketHandler>(
        *connection_manager_, *packet_sender_, *network_);
    packet_handler_->RegisterHandlers();

    // Authenticate two clients (fill server)
    AddAuthenticatedClient("Player1");
    AddAuthenticatedClient("Player2");

    // Try to authenticate third client
    uint32_t client_id3 = AddTestClient();

    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("Player3");

    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    packet_handler_->Dispatch(client_id3, result);

    // Verify authentication failed
    auto &client3 = connection_manager_->GetClient(client_id3);
    EXPECT_FALSE(client3.IsAuthenticated());
    EXPECT_EQ(client3.player_id_, 0);
}

TEST_F(PacketHandlerTest, HandleConnectReqWhitespaceUsername) {
    uint32_t client_id = AddTestClient();

    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("  TrimmedName  ");

    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    packet_handler_->Dispatch(client_id, result);

    // Verify authentication succeeded and username was trimmed
    auto &client = connection_manager_->GetClient(client_id);
    EXPECT_TRUE(client.IsAuthenticated());
    EXPECT_EQ(client.username_, "TrimmedName");
}

// ============================================================================
// READY_STATUS HANDLING TESTS
// ============================================================================

TEST_F(PacketHandlerTest, HandleReadyStatusSetReady) {
    uint32_t client_id = AddAuthenticatedClient("Player1");
    auto &client = connection_manager_->GetClient(client_id);

    EXPECT_FALSE(client.ready_);

    // Create READY_STATUS packet (is_ready = 1)
    server::network::ReadyStatusPacket ready_packet;
    ready_packet.is_ready = 1;

    server::network::PacketParseResult result;
    result.success = true;
    result.header = ready_packet.MakeHeader();
    result.packet = ready_packet;

    packet_handler_->Dispatch(client_id, result);

    // Verify ready status was set
    EXPECT_TRUE(client.ready_);
}

TEST_F(PacketHandlerTest, HandleReadyStatusSetNotReady) {
    uint32_t client_id = AddAuthenticatedClient("Player1");
    auto &client = connection_manager_->GetClient(client_id);

    // Set ready first
    client.ready_ = true;

    // Create READY_STATUS packet (is_ready = 0)
    server::network::ReadyStatusPacket ready_packet;
    ready_packet.is_ready = 0;

    server::network::PacketParseResult result;
    result.success = true;
    result.header = ready_packet.MakeHeader();
    result.packet = ready_packet;

    packet_handler_->Dispatch(client_id, result);

    // Verify ready status was unset
    EXPECT_FALSE(client.ready_);
}

TEST_F(PacketHandlerTest, HandleReadyStatusUnauthenticatedClient) {
    uint32_t client_id = AddTestClient();  // Not authenticated

    server::network::ReadyStatusPacket ready_packet;
    ready_packet.is_ready = 1;

    server::network::PacketParseResult result;
    result.success = true;
    result.header = ready_packet.MakeHeader();
    result.packet = ready_packet;

    // Should not crash or change ready status
    EXPECT_NO_THROW(packet_handler_->Dispatch(client_id, result));

    auto &client = connection_manager_->GetClient(client_id);
    EXPECT_FALSE(client.ready_);  // Should remain false
}

TEST_F(PacketHandlerTest, HandleReadyStatusGameStartCallback) {
    // Add two players
    uint32_t client_id1 = AddAuthenticatedClient("Player1");
    uint32_t client_id2 = AddAuthenticatedClient("Player2");

    bool game_started = false;
    packet_handler_->SetGameStartCallback(
        [&game_started]() { game_started = true; });

    // Set first player ready
    server::network::ReadyStatusPacket ready_packet1;
    ready_packet1.is_ready = 1;
    server::network::PacketParseResult result1;
    result1.success = true;
    result1.header = ready_packet1.MakeHeader();
    result1.packet = ready_packet1;

    packet_handler_->Dispatch(client_id1, result1);

    // Game should not start yet
    EXPECT_FALSE(game_started);

    // Set second player ready
    server::network::ReadyStatusPacket ready_packet2;
    ready_packet2.is_ready = 1;
    server::network::PacketParseResult result2;
    result2.success = true;
    result2.header = ready_packet2.MakeHeader();
    result2.packet = ready_packet2;

    packet_handler_->Dispatch(client_id2, result2);

    // Game should start now
    EXPECT_TRUE(game_started);
}

TEST_F(PacketHandlerTest, HandleReadyStatusNoCallbackNoSegfault) {
    uint32_t client_id = AddAuthenticatedClient("Player1");

    // Don't set callback
    server::network::ReadyStatusPacket ready_packet;
    ready_packet.is_ready = 1;

    server::network::PacketParseResult result;
    result.success = true;
    result.header = ready_packet.MakeHeader();
    result.packet = ready_packet;

    // Should not crash when callback is null
    EXPECT_NO_THROW(packet_handler_->Dispatch(client_id, result));
}

// ============================================================================
// DISCONNECT_REQ HANDLING TESTS
// ============================================================================

TEST_F(PacketHandlerTest, HandleDisconnectReqAuthenticatedClient) {
    uint32_t client_id = AddAuthenticatedClient("Player1");

    EXPECT_TRUE(connection_manager_->HasClient(client_id));

    server::network::DisconnectReqPacket disconnect_packet;
    server::network::PacketParseResult result;
    result.success = true;
    result.header = disconnect_packet.MakeHeader();
    result.packet = disconnect_packet;

    packet_handler_->Dispatch(client_id, result);

    // Verify client was removed
    EXPECT_FALSE(connection_manager_->HasClient(client_id));
}

TEST_F(PacketHandlerTest, HandleDisconnectReqUnauthenticatedClient) {
    uint32_t client_id = AddTestClient();

    EXPECT_TRUE(connection_manager_->HasClient(client_id));

    server::network::DisconnectReqPacket disconnect_packet;
    server::network::PacketParseResult result;
    result.success = true;
    result.header = disconnect_packet.MakeHeader();
    result.packet = disconnect_packet;

    packet_handler_->Dispatch(client_id, result);

    // Verify client was removed
    EXPECT_FALSE(connection_manager_->HasClient(client_id));
}

// ============================================================================
// DISPATCH TESTS
// ============================================================================

TEST_F(PacketHandlerTest, DispatchValidPacket) {
    uint32_t client_id = AddTestClient();

    // Create a valid CONNECT_REQ packet
    server::network::ConnectReqPacket connect_req;
    connect_req.SetUsername("TestPlayer");

    server::network::PacketParseResult result;
    result.success = true;
    result.header = connect_req.MakeHeader();
    result.packet = connect_req;

    // Should not crash
    EXPECT_NO_THROW(packet_handler_->Dispatch(client_id, result));

    // Verify packet was handled (client should be authenticated)
    auto &client = connection_manager_->GetClient(client_id);
    EXPECT_TRUE(client.IsAuthenticated());
}

TEST_F(PacketHandlerTest, DispatchInvalidPacket) {
    uint32_t client_id = AddTestClient();

    server::network::PacketParseResult result;
    result.success = false;
    result.error = "Test error";

    // Should not crash, should log error
    EXPECT_NO_THROW(packet_handler_->Dispatch(client_id, result));
}

TEST_F(PacketHandlerTest, DispatchNonExistentClient) {
    server::network::PacketParseResult result;
    result.success = true;

    // Should not crash when client doesn't exist
    EXPECT_NO_THROW(packet_handler_->Dispatch(999, result));
}

TEST_F(PacketHandlerTest, DispatchUnhandledPacketType) {
    uint32_t client_id = AddTestClient();

    server::network::PacketParseResult result;
    result.success = true;
    result.header.op_code = 0xFF;  // Invalid/unhandled packet type

    // Should not crash, should log error
    EXPECT_NO_THROW(packet_handler_->Dispatch(client_id, result));
}

// ============================================================================
// AUTOSTART FUNCTIONALITY TESTS
// ============================================================================

TEST_F(PacketHandlerTest, AutostartSinglePlayer) {
    uint32_t client_id = AddAuthenticatedClient("Player1");

    bool game_started = false;
    packet_handler_->SetGameStartCallback(
        [&game_started]() { game_started = true; });

    // Set player ready
    server::network::ReadyStatusPacket ready_packet;
    ready_packet.is_ready = 1;
    server::network::PacketParseResult result;
    result.success = true;
    result.header = ready_packet.MakeHeader();
    result.packet = ready_packet;

    packet_handler_->Dispatch(client_id, result);

    // Game should start with single player
    EXPECT_TRUE(game_started);
}

TEST_F(PacketHandlerTest, AutostartMultiplePlayers) {
    // Add 3 players
    uint32_t client_id1 = AddAuthenticatedClient("Player1");
    uint32_t client_id2 = AddAuthenticatedClient("Player2");
    uint32_t client_id3 = AddAuthenticatedClient("Player3");

    bool game_started = false;
    packet_handler_->SetGameStartCallback(
        [&game_started]() { game_started = true; });

    // Set first two players ready
    server::network::ReadyStatusPacket ready_packet1;
    ready_packet1.is_ready = 1;
    server::network::PacketParseResult result1;
    result1.success = true;
    result1.header = ready_packet1.MakeHeader();
    result1.packet = ready_packet1;
    packet_handler_->Dispatch(client_id1, result1);

    server::network::ReadyStatusPacket ready_packet2;
    ready_packet2.is_ready = 1;
    server::network::PacketParseResult result2;
    result2.success = true;
    result2.header = ready_packet2.MakeHeader();
    result2.packet = ready_packet2;
    packet_handler_->Dispatch(client_id2, result2);

    // Game should not start yet
    EXPECT_FALSE(game_started);

    // Set third player ready
    server::network::ReadyStatusPacket ready_packet3;
    ready_packet3.is_ready = 1;
    server::network::PacketParseResult result3;
    result3.success = true;
    result3.header = ready_packet3.MakeHeader();
    result3.packet = ready_packet3;
    packet_handler_->Dispatch(client_id3, result3);

    // Game should start now
    EXPECT_TRUE(game_started);
}

TEST_F(PacketHandlerTest, AutostartNoPlayersConnected) {
    bool game_started = false;
    packet_handler_->SetGameStartCallback(
        [&game_started]() { game_started = true; });

    // No players connected - game should not start
    EXPECT_FALSE(connection_manager_->AllPlayersReady());
    EXPECT_FALSE(game_started);
}

TEST_F(PacketHandlerTest, AutostartOnlyUnauthenticatedClients) {
    AddTestClient();  // Unauthenticated

    bool game_started = false;
    packet_handler_->SetGameStartCallback(
        [&game_started]() { game_started = true; });

    // Only unauthenticated clients - game should not start
    EXPECT_FALSE(connection_manager_->AllPlayersReady());
    EXPECT_FALSE(game_started);
}
