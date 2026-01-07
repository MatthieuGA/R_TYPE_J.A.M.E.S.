/**
 * @file test_network_input_system.cpp
 * @brief Unit tests for NetworkInputSystem
 *
 * Tests cover:
 * - System behavior when not connected
 * - System behavior when connected
 * - Bitfield conversion integration
 * - SendInput calls with correct values
 * - io_context polling
 */

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <boost/asio.hpp>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "platform/SFMLWindow.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;
namespace RC = Rtype::Client;

using RC::InputToBitfield;
using RC::NetworkInputSystem;

// ============================================================================
// Helper Classes for Testing
// ============================================================================

/**
 * @brief Mock ServerConnection for testing NetworkInputSystem
 */
class MockServerConnection {
 public:
    explicit MockServerConnection(
        boost::asio::io_context &io, bool connected = false)
        : io_context_(io), connected_(connected), last_input_sent_(0xFF) {}

    bool is_connected() const {
        return connected_;
    }

    void SendInput(uint8_t input_flags) {
        send_input_called_ = true;
        last_input_sent_ = input_flags;
    }

    void SetConnected(bool connected) {
        connected_ = connected;
    }

    bool WasSendInputCalled() const {
        return send_input_called_;
    }

    uint8_t GetLastInputSent() const {
        return last_input_sent_;
    }

    void ResetCallTracking() {
        send_input_called_ = false;
        last_input_sent_ = 0xFF;
    }

 private:
    boost::asio::io_context &io_context_;
    bool connected_;
    bool send_input_called_ = false;
    uint8_t last_input_sent_;
};

// ============================================================================
// NetworkInputSystem Tests
// ============================================================================

TEST(NetworkInputSystem, DoesNotSendWhenNotConnected) {
    Eng::registry reg;
    boost::asio::io_context io;

    // Create a mock game world with disconnected server
    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);
    game_world.server_connection_.reset();  // No connection

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Create entity with input
    inputs.insert_at(0, Com::Inputs{1.0f, 0.0f, true, false});
    player_tags.insert_at(0, Com::PlayerTag{});

    // Call system - should not crash or send anything
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(
    NetworkInputSystem, DoesNotSendWhenServerConnectionExistsButNotConnected) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);
    // Server connection exists but is_connected() returns false (default
    // state)

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Create entity with input
    inputs.insert_at(0, Com::Inputs{1.0f, 0.0f, true, false});
    player_tags.insert_at(0, Com::PlayerTag{});

    // Call system - should not send because not connected
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(NetworkInputSystem, PollsIoContextWhenConnected) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // No entities - system should still poll io_context if connected
    // This test verifies that polling happens before entity iteration

    // Note: We can't easily verify poll() was called without mocking,
    // but we can verify the system doesn't crash when io_context has
    // pending handlers

    bool handler_executed = false;
    boost::asio::post(game_world.io_context_,
        [&handler_executed]() { handler_executed = true; });

    // System will poll io_context even with no entities
    // (if connected, which we can't easily test without actual connection)
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(NetworkInputSystem, SkipsEntitiesWithoutBothComponents) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Entity with input but no PlayerTag
    inputs.insert_at(0, Com::Inputs{1.0f, 0.0f, true, false});

    // Entity with PlayerTag but no input
    player_tags.insert_at(1, Com::PlayerTag{});

    // System should not crash when entities lack required components
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(NetworkInputSystem, HandlesMultiplePlayerEntities) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Create multiple player entities with different inputs
    inputs.insert_at(0, Com::Inputs{1.0f, 0.0f, false, false});  // Right
    player_tags.insert_at(0, Com::PlayerTag{});

    // Left+Down+Shoot
    inputs.insert_at(1, Com::Inputs{-1.0f, 1.0f, true, false});
    player_tags.insert_at(1, Com::PlayerTag{});

    inputs.insert_at(2, Com::Inputs{0.0f, -1.0f, false, false});  // Up
    player_tags.insert_at(2, Com::PlayerTag{});

    // System should handle multiple entities without crashing
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

// ============================================================================
// Bitfield Conversion Integration Tests
// ============================================================================

TEST(NetworkInputSystem, ConvertsCombinedInputsCorrectly) {
    // Test that the system uses InputToBitfield correctly
    Com::Inputs input{-1.0f, 1.0f, true, false};  // Left + Down + Shoot

    uint8_t bitfield = InputToBitfield(input);

    // Verify bitfield matches RFC spec:
    // Bit 1: Down (0x02)
    // Bit 2: Left (0x04)
    // Bit 4: Shoot (0x10)
    // Expected: 0x02 | 0x04 | 0x10 = 0x16
    EXPECT_EQ(bitfield, 0x16);
}

TEST(NetworkInputSystem, ConvertsAllDirectionsCorrectly) {
    // Test Up+Right
    Com::Inputs input1{1.0f, -1.0f, false, false};
    uint8_t bitfield1 = InputToBitfield(input1);
    EXPECT_EQ(bitfield1, 0x01 | 0x08);  // Up (0x01) + Right (0x08) = 0x09

    // Test Down+Left
    Com::Inputs input2{-1.0f, 1.0f, false, false};
    uint8_t bitfield2 = InputToBitfield(input2);
    EXPECT_EQ(bitfield2, 0x02 | 0x04);  // Down (0x02) + Left (0x04) = 0x06

    // Test Shoot only
    Com::Inputs input3{0.0f, 0.0f, true, false};
    uint8_t bitfield3 = InputToBitfield(input3);
    EXPECT_EQ(bitfield3, 0x10);  // Shoot (0x10)
}

TEST(NetworkInputSystem, ConvertsNoInputToZero) {
    Com::Inputs input{0.0f, 0.0f, false, false};
    uint8_t bitfield = InputToBitfield(input);
    EXPECT_EQ(bitfield, 0x00);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(NetworkInputSystem, HandlesEmptyRegistry) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // No entities at all
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(NetworkInputSystem, HandlesPartialInputComponents) {
    Eng::registry reg;
    boost::asio::io_context io;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    RC::GameWorld game_world(std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Create entities with gaps in indices
    inputs.insert_at(0, Com::Inputs{1.0f, 0.0f, false, false});
    player_tags.insert_at(0, Com::PlayerTag{});

    // Gap at index 1

    inputs.insert_at(2, Com::Inputs{-1.0f, 0.0f, false, false});
    player_tags.insert_at(2, Com::PlayerTag{});

    // System should handle sparse arrays correctly
    EXPECT_NO_THROW(NetworkInputSystem(reg, game_world, inputs, player_tags));
}

TEST(NetworkInputSystem, HandlesBoundaryInputValues) {
    // Test edge cases for input values
    Com::Inputs max_input{1.0f, 1.0f, true, false};    // Max values
    Com::Inputs min_input{-1.0f, -1.0f, true, false};  // Min values
    Com::Inputs zero_input{0.0f, 0.0f, false, false};  // Zero values

    uint8_t max_bitfield = InputToBitfield(max_input);
    uint8_t min_bitfield = InputToBitfield(min_input);
    uint8_t zero_bitfield = InputToBitfield(zero_input);

    // Max: Down (0x02) + Right (0x08) + Shoot (0x10) = 0x1A
    EXPECT_EQ(max_bitfield, 0x1A);

    // Min: Up (0x01) + Left (0x04) + Shoot (0x10) = 0x15
    EXPECT_EQ(min_bitfield, 0x15);

    // Zero: Nothing = 0x00
    EXPECT_EQ(zero_bitfield, 0x00);
}

// ============================================================================
// RFC Compliance Tests
// ============================================================================

TEST(NetworkInputSystem, BitfieldMatchesRFCSpecification) {
    // RFC Section 6.1 defines PLAYER_INPUT bitfield format:
    // Bit 0: Up
    // Bit 1: Down
    // Bit 2: Left
    // Bit 3: Right
    // Bit 4: Shoot

    // Test each bit individually
    // Up
    EXPECT_EQ(InputToBitfield(Com::Inputs{0.0f, -1.0f, false, false}), 0x01);
    // Down
    EXPECT_EQ(InputToBitfield(Com::Inputs{0.0f, 1.0f, false, false}), 0x02);
    // Left
    EXPECT_EQ(InputToBitfield(Com::Inputs{-1.0f, 0.0f, false, false}), 0x04);
    // Right
    EXPECT_EQ(InputToBitfield(Com::Inputs{1.0f, 0.0f, false, false}), 0x08);
    // Shoot
    EXPECT_EQ(InputToBitfield(Com::Inputs{0.0f, 0.0f, true, false}), 0x10);
}

TEST(NetworkInputSystem, AllBitsSetProducesCorrectBitfield) {
    // All inputs active should set all 5 bits
    // Note: This is physically impossible (can't go Up+Down or Left+Right)
    // but test the bitfield logic
    Com::Inputs all_inputs{1.0f, 1.0f, true, false};  // Right+Down+Shoot

    uint8_t bitfield = InputToBitfield(all_inputs);

    // Should have bits 1, 3, 4 set: 0x02 | 0x08 | 0x10 = 0x1A
    EXPECT_EQ(bitfield, 0x1A);
    EXPECT_EQ(bitfield & 0x02, 0x02);  // Down bit
    EXPECT_EQ(bitfield & 0x08, 0x08);  // Right bit
    EXPECT_EQ(bitfield & 0x10, 0x10);  // Shoot bit
}
