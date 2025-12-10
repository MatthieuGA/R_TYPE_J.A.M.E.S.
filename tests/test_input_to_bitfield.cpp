#include <gtest/gtest.h>

#include <cstdint>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"

namespace Com = Rtype::Client::Component;

using Rtype::Client::InputToBitfield;

TEST(InputToBitfield, AllInputsZero) {
    Com::Inputs input{};
    input.horizontal = 0.0f;
    input.vertical = 0.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x00);
}

TEST(InputToBitfield, UpOnly) {
    Com::Inputs input{};
    input.horizontal = 0.0f;
    input.vertical = -1.0f;  // Up (negative vertical)
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x01);  // Bit 0
}

TEST(InputToBitfield, DownOnly) {
    Com::Inputs input{};
    input.horizontal = 0.0f;
    input.vertical = 1.0f;  // Down (positive vertical)
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x02);  // Bit 1
}

TEST(InputToBitfield, LeftOnly) {
    Com::Inputs input{};
    input.horizontal = -1.0f;  // Left (negative horizontal)
    input.vertical = 0.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x04);  // Bit 2
}

TEST(InputToBitfield, RightOnly) {
    Com::Inputs input{};
    input.horizontal = 1.0f;  // Right (positive horizontal)
    input.vertical = 0.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x08);  // Bit 3
}

TEST(InputToBitfield, ShootOnly) {
    Com::Inputs input{};
    input.horizontal = 0.0f;
    input.vertical = 0.0f;
    input.shoot = true;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x10);  // Bit 4
}

TEST(InputToBitfield, UpAndLeft) {
    Com::Inputs input{};
    input.horizontal = -1.0f;
    input.vertical = -1.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x05);  // Bit 0 | Bit 2
}

TEST(InputToBitfield, DownAndRight) {
    Com::Inputs input{};
    input.horizontal = 1.0f;
    input.vertical = 1.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x0A);  // Bit 1 | Bit 3
}

TEST(InputToBitfield, AllDirectionsAndShoot) {
    // This is an impossible state (can't press both up and down),
    // but we test the bitfield logic
    Com::Inputs input{};
    input.horizontal = 1.0f;  // Right
    input.vertical = 1.0f;    // Down
    input.shoot = true;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x1A);  // Bit 1 | Bit 3 | Bit 4
}

TEST(InputToBitfield, ShootWithMovement) {
    Com::Inputs input{};
    input.horizontal = -1.0f;  // Left
    input.vertical = -1.0f;    // Up
    input.shoot = true;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x15);  // Bit 0 | Bit 2 | Bit 4
}

TEST(InputToBitfield, PartialMovementValues) {
    // Test with non-normalized values (0.5, 0.7, etc.)
    Com::Inputs input{};
    input.horizontal = 0.5f;  // Positive (right)
    input.vertical = -0.3f;   // Negative (up)
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x09);  // Bit 0 | Bit 3
}

TEST(InputToBitfield, VerySmallValues) {
    // Test edge case with very small but non-zero values
    Com::Inputs input{};
    input.horizontal = 0.001f;  // Positive (right)
    input.vertical = -0.001f;   // Negative (up)
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x09);  // Bit 0 | Bit 3
}

TEST(InputToBitfield, ExactlyZeroNotSet) {
    // Ensure exactly 0.0 doesn't set any direction bits
    Com::Inputs input{};
    input.horizontal = 0.0f;
    input.vertical = 0.0f;
    input.shoot = true;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x10);  // Only Bit 4 (shoot)
}

TEST(InputToBitfield, NegativeZeroTreatedAsZero) {
    Com::Inputs input{};
    input.horizontal = -0.0f;
    input.vertical = -0.0f;
    input.shoot = false;

    uint8_t bitfield = InputToBitfield(input);

    EXPECT_EQ(bitfield, 0x00);
}

TEST(InputToBitfield, MatchesRFCSpecification) {
    // RFC Section 6.1: PLAYER_INPUT bitfield
    // Bit 0: Up, Bit 1: Down, Bit 2: Left, Bit 3: Right, Bit 4: Shoot

    Com::Inputs up{0.0f, -1.0f, false, false};
    EXPECT_EQ(InputToBitfield(up), 1 << 0);

    Com::Inputs down{0.0f, 1.0f, false, false};
    EXPECT_EQ(InputToBitfield(down), 1 << 1);

    Com::Inputs left{-1.0f, 0.0f, false, false};
    EXPECT_EQ(InputToBitfield(left), 1 << 2);

    Com::Inputs right{1.0f, 0.0f, false, false};
    EXPECT_EQ(InputToBitfield(right), 1 << 3);

    Com::Inputs shoot{0.0f, 0.0f, true, false};
    EXPECT_EQ(InputToBitfield(shoot), 1 << 4);
}
