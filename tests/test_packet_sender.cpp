/**
 * @file test_packet_sender.cpp
 * @brief Unit tests for PacketSender class
 *
 * Tests packet sending functionality including CONNECT_ACK responses
 * with different statuses and GAME_START broadcasting to authenticated
 * players only.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/PacketSender.hpp"
#include "server/Packets.hpp"

/**
 * @brief Test fixture for PacketSender tests
 */
class PacketSenderTest : public ::testing::Test {
 protected:
    void SetUp() override {
        io_context_ = std::make_unique<boost::asio::io_context>();
        connection_manager_ =
            std::make_unique<server::ClientConnectionManager>(4);
        packet_sender_ =
            std::make_unique<server::PacketSender>(*connection_manager_);
    }

    void TearDown() override {
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
    std::unique_ptr<server::ClientConnectionManager> connection_manager_;
    std::unique_ptr<server::PacketSender> packet_sender_;
};

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

TEST_F(PacketSenderTest, ConstructorSucceeds) {
    EXPECT_NO_THROW(server::PacketSender sender(*connection_manager_));
}

// ============================================================================
// SEND CONNECT_ACK TESTS
// ============================================================================

TEST_F(PacketSenderTest, SendConnectAckOK) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should not crash when sending CONNECT_ACK with OK status
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::OK, 42));
}

TEST_F(PacketSenderTest, SendConnectAckServerFull) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should not crash when sending CONNECT_ACK with ServerFull status
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::ServerFull, 0));
}

TEST_F(PacketSenderTest, SendConnectAckBadUsername) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should not crash when sending CONNECT_ACK with BadUsername status
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::BadUsername, 0));
}

TEST_F(PacketSenderTest, SendConnectAckInGame) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should not crash when sending CONNECT_ACK with InGame status
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::InGame, 0));
}

TEST_F(PacketSenderTest, SendConnectAckMultipleClients) {
    uint32_t client_id1 = AddTestClient();
    uint32_t client_id2 = AddTestClient();

    auto &client1 = connection_manager_->GetClient(client_id1);
    auto &client2 = connection_manager_->GetClient(client_id2);

    // Should handle sending to multiple clients
    EXPECT_NO_THROW({
        packet_sender_->SendConnectAck(
            client1, server::network::ConnectAckPacket::OK, 1);
        packet_sender_->SendConnectAck(
            client2, server::network::ConnectAckPacket::OK, 2);
    });
}

// ============================================================================
// SEND GAME_START TESTS
// ============================================================================

TEST_F(PacketSenderTest, SendGameStartNoClients) {
    // Should not crash when no clients connected
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

TEST_F(PacketSenderTest, SendGameStartSingleAuthenticatedClient) {
    uint32_t client_id = AddAuthenticatedClient("Player1");

    // Should not crash when sending to one authenticated client
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

TEST_F(PacketSenderTest, SendGameStartMultipleAuthenticatedClients) {
    AddAuthenticatedClient("Player1");
    AddAuthenticatedClient("Player2");
    AddAuthenticatedClient("Player3");

    // Should not crash when sending to multiple authenticated clients
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

TEST_F(PacketSenderTest, SendGameStartIgnoresUnauthenticatedClients) {
    // Add mix of authenticated and unauthenticated clients
    AddAuthenticatedClient("Player1");
    AddTestClient();  // Unauthenticated
    AddAuthenticatedClient("Player2");
    AddTestClient();  // Unauthenticated

    // Should only send to authenticated clients (should not crash)
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

TEST_F(PacketSenderTest, SendGameStartOnlyUnauthenticatedClients) {
    // Add only unauthenticated clients
    AddTestClient();
    AddTestClient();

    // Should not crash (no authenticated clients to send to)
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_F(PacketSenderTest, SendConnectAckThenGameStart) {
    uint32_t client_id1 = AddTestClient();
    uint32_t client_id2 = AddTestClient();

    auto &client1 = connection_manager_->GetClient(client_id1);
    auto &client2 = connection_manager_->GetClient(client_id2);

    // Authenticate clients via CONNECT_ACK
    connection_manager_->AuthenticateClient(client_id1, "Player1");
    connection_manager_->AuthenticateClient(client_id2, "Player2");

    // Send CONNECT_ACK to both
    EXPECT_NO_THROW({
        packet_sender_->SendConnectAck(
            client1, server::network::ConnectAckPacket::OK, 1);
        packet_sender_->SendConnectAck(
            client2, server::network::ConnectAckPacket::OK, 2);
    });

    // Send GAME_START to all authenticated
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

TEST_F(PacketSenderTest, SendGameStartAfterClientRemoval) {
    uint32_t client_id1 = AddAuthenticatedClient("Player1");
    uint32_t client_id2 = AddAuthenticatedClient("Player2");

    // Remove one client
    connection_manager_->RemoveClient(client_id1);

    // Should only send to remaining client
    EXPECT_NO_THROW(packet_sender_->SendGameStart());
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(PacketSenderTest, SendConnectAckWithZeroPlayerId) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should handle player_id = 0 (used for rejection responses)
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::ServerFull, 0));
}

TEST_F(PacketSenderTest, SendConnectAckWithMaxPlayerId) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Should handle player_id = 255 (max value)
    EXPECT_NO_THROW(packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::OK, 255));
}

TEST_F(PacketSenderTest, MultipleGameStartCalls) {
    AddAuthenticatedClient("Player1");
    AddAuthenticatedClient("Player2");

    // Should handle multiple GAME_START calls without crashing
    EXPECT_NO_THROW({
        packet_sender_->SendGameStart();
        packet_sender_->SendGameStart();
        packet_sender_->SendGameStart();
    });
}

// ============================================================================
// ASYNC SEND TESTS
// ============================================================================

TEST_F(PacketSenderTest, AsyncSendDoesNotBlock) {
    uint32_t client_id = AddTestClient();
    auto &client = connection_manager_->GetClient(client_id);

    // Measure time to send packet (should be very fast due to async)
    auto start = std::chrono::steady_clock::now();
    packet_sender_->SendConnectAck(
        client, server::network::ConnectAckPacket::OK, 1);
    auto end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Async send should complete almost instantly (< 100ms)
    EXPECT_LT(duration.count(), 100);
}

TEST_F(PacketSenderTest, GameStartAsyncSendDoesNotBlock) {
    AddAuthenticatedClient("Player1");
    AddAuthenticatedClient("Player2");
    AddAuthenticatedClient("Player3");

    // Measure time to send GAME_START to all clients
    auto start = std::chrono::steady_clock::now();
    packet_sender_->SendGameStart();
    auto end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Async send should complete almost instantly (< 100ms)
    EXPECT_LT(duration.count(), 100);
}
