#include <gtest/gtest.h>

#include "server/PacketBuffer.hpp"
#include "server/PacketFactory.hpp"
#include "server/PacketTypes.hpp"
#include "server/Packets.hpp"

using namespace server::network;

// ============================================================================
// RFC COMPLIANCE VERIFICATION TESTS
// ============================================================================

TEST(RFCComplianceTest, CommonHeaderSize) {
    // RFC Section 4.1: Header MUST be exactly 12 bytes
    EXPECT_EQ(sizeof(CommonHeader), 12);
}

TEST(RFCComplianceTest, OpcodeSize) {
    // RFC Section 3.1: OpCode is u8 (1 byte)
    EXPECT_EQ(sizeof(PacketType), 1);
}

TEST(RFCComplianceTest, PlayerIdSize) {
    // RFC Section 3.1: PlayerId is u8 (1 byte)
    EXPECT_EQ(sizeof(PlayerId), 1);
}

TEST(RFCComplianceTest, OpcodeRanges) {
    // TCP opcodes: 0x01-0x07
    EXPECT_EQ(static_cast<uint8_t>(PacketType::ConnectReq), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::ConnectAck), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::DisconnectReq), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::NotifyDisconnect), 0x04);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::GameStart), 0x05);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::GameEnd), 0x06);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::ReadyStatus), 0x07);

    // UDP opcodes: 0x10+
    EXPECT_EQ(static_cast<uint8_t>(PacketType::PlayerInput), 0x10);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::WorldSnapshot), 0x20);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::PlayerStats), 0x21);
}

TEST(RFCComplianceTest, InputFlagBitmask) {
    // RFC Section 6.1: Input bitmask values
    EXPECT_EQ(InputFlags::UP, 0x01);
    EXPECT_EQ(InputFlags::DOWN, 0x02);
    EXPECT_EQ(InputFlags::LEFT, 0x04);
    EXPECT_EQ(InputFlags::RIGHT, 0x08);
    EXPECT_EQ(InputFlags::SHOOT, 0x10);
}

// ============================================================================
// PACKET BUFFER TESTS
// ============================================================================

TEST(PacketBufferTest, WriteAndReadUint8) {
    PacketBuffer buffer;
    buffer.write_uint8(42);
    buffer.write_uint8(255);

    EXPECT_EQ(buffer.size(), 2);
    EXPECT_EQ(buffer.read_uint8(), 42);
    EXPECT_EQ(buffer.read_uint8(), 255);
}

TEST(PacketBufferTest, WriteAndReadUint16) {
    PacketBuffer buffer;
    buffer.write_uint16(0x1234);
    buffer.write_uint16(0xFFFF);

    EXPECT_EQ(buffer.read_uint16(), 0x1234);
    EXPECT_EQ(buffer.read_uint16(), 0xFFFF);
}

TEST(PacketBufferTest, WriteAndReadUint32) {
    PacketBuffer buffer;
    buffer.write_uint32(0x12345678);
    buffer.write_uint32(0xFFFFFFFF);

    EXPECT_EQ(buffer.read_uint32(), 0x12345678);
    EXPECT_EQ(buffer.read_uint32(), 0xFFFFFFFF);
}

TEST(PacketBufferTest, ReadBeyondBoundsThrows) {
    PacketBuffer buffer;
    buffer.write_uint8(42);

    buffer.read_uint8();  // OK
    EXPECT_THROW(buffer.read_uint8(), std::out_of_range);
}

TEST(PacketBufferTest, ResetReadOffset) {
    PacketBuffer buffer;
    buffer.write_uint8(42);
    buffer.write_uint8(100);

    EXPECT_EQ(buffer.read_uint8(), 42);
    buffer.reset_read_offset();
    EXPECT_EQ(buffer.read_uint8(), 42);
}

// ============================================================================
// COMMON HEADER TESTS
// ============================================================================

TEST(CommonHeaderTest, DefaultConstruction) {
    CommonHeader header;

    EXPECT_EQ(header.op_code, 0);
    EXPECT_EQ(header.payload_size, 0);
    EXPECT_EQ(header.packet_index, 0);
    EXPECT_EQ(header.tick_id, 0);
    EXPECT_EQ(header.packet_count, 1);
    EXPECT_EQ(header.reserved[0], 0);
    EXPECT_EQ(header.reserved[1], 0);
    EXPECT_EQ(header.reserved[2], 0);
}

TEST(CommonHeaderTest, SerializeDeserialize) {
    CommonHeader original(0x05, 128, 9999, 2, 5);

    PacketBuffer buffer;
    buffer.write_header(original);

    EXPECT_EQ(buffer.size(), 12);

    CommonHeader deserialized = buffer.read_header();

    EXPECT_EQ(deserialized.op_code, original.op_code);
    EXPECT_EQ(deserialized.payload_size, original.payload_size);
    EXPECT_EQ(deserialized.packet_index, original.packet_index);
    EXPECT_EQ(deserialized.tick_id, original.tick_id);
    EXPECT_EQ(deserialized.packet_count, original.packet_count);
}

// ============================================================================
// STRONG TYPES TESTS
// ============================================================================

TEST(StrongTypesTest, PlayerId) {
    PlayerId id1{42};
    PlayerId id2{42};
    PlayerId id3{100};

    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
    EXPECT_EQ(id1.value, 42);
}

TEST(StrongTypesTest, EntityId) {
    EntityId id1{1000};
    EntityId id2{1000};

    EXPECT_EQ(id1, id2);
    EXPECT_EQ(id1.value, 1000);
}

TEST(StrongTypesTest, InputFlags) {
    InputFlags flags{0};

    EXPECT_FALSE(flags.has(InputFlags::UP));

    flags.set(InputFlags::UP);
    EXPECT_TRUE(flags.has(InputFlags::UP));

    flags.set(InputFlags::SHOOT);
    EXPECT_TRUE(flags.has(InputFlags::UP));
    EXPECT_TRUE(flags.has(InputFlags::SHOOT));

    flags.clear(InputFlags::UP);
    EXPECT_FALSE(flags.has(InputFlags::UP));
    EXPECT_TRUE(flags.has(InputFlags::SHOOT));
}

// ============================================================================
// TCP PACKET TESTS
// ============================================================================

TEST(TCPPacketTest, ConnectReqPacketSize) {
    ConnectReqPacket packet;
    packet.set_username("TestPlayer");

    PacketBuffer buffer;
    packet.serialize(buffer);

    // 12 byte header + 32 byte payload = 44 bytes
    EXPECT_EQ(buffer.size(), 44);
}

TEST(TCPPacketTest, ConnectReqRoundTrip) {
    ConnectReqPacket original;
    original.set_username("Alice");

    PacketBuffer buffer;
    original.serialize(buffer);

    buffer.reset_read_offset();
    CommonHeader header = buffer.read_header();
    EXPECT_EQ(header.op_code, 0x01);
    EXPECT_EQ(header.tick_id, 0);  // TCP packets have tick_id = 0

    auto deserialized = ConnectReqPacket::deserialize(buffer);
    EXPECT_EQ(deserialized.get_username(), "Alice");
}

TEST(TCPPacketTest, ConnectAckStatusCodes) {
    EXPECT_EQ(ConnectAckPacket::OK, 0);
    EXPECT_EQ(ConnectAckPacket::ServerFull, 1);
    EXPECT_EQ(ConnectAckPacket::BadUsername, 2);
    EXPECT_EQ(ConnectAckPacket::InGame, 3);
}

TEST(TCPPacketTest, DisconnectReqPacketSize) {
    DisconnectReqPacket packet;

    PacketBuffer buffer;
    packet.serialize(buffer);

    // 12 byte header + 0 byte payload = 12 bytes
    EXPECT_EQ(buffer.size(), 12);
}

TEST(TCPPacketTest, GameEndDrawScenario) {
    GameEndPacket packet;
    packet.winning_player_id = PlayerId{0};  // 0 = draw
    packet.reserved = {0, 0, 0};

    PacketBuffer buffer;
    packet.serialize(buffer);

    buffer.reset_read_offset();
    buffer.read_header();

    auto deserialized = GameEndPacket::deserialize(buffer);
    EXPECT_EQ(deserialized.winning_player_id.value, 0);
}

// ============================================================================
// UDP PACKET TESTS
// ============================================================================

TEST(UDPPacketTest, PlayerInputWithTickId) {
    PlayerInputPacket packet;
    packet.inputs = InputFlags{InputFlags::RIGHT};
    packet.reserved = {0, 0, 0};

    PacketBuffer buffer;
    packet.serialize(buffer, 99999);

    buffer.reset_read_offset();
    CommonHeader header = buffer.read_header();
    EXPECT_EQ(header.op_code, 0x10);
    EXPECT_EQ(header.tick_id, 99999);
}

TEST(UDPPacketTest, WorldSnapshotWithEntities) {
    WorldSnapshotPacket packet;
    packet.entity_count = 2;
    packet.reserved = {0, 0};

    EntityState e1;
    e1.entity_id = EntityId{100};
    e1.entity_type = 1;
    e1.reserved = 0;
    e1.pos_x = 32767;
    e1.pos_y = 19432;
    e1.angle = 180;
    packet.entities.push_back(e1);

    EntityState e2;
    e2.entity_id = EntityId{101};
    e2.entity_type = 2;
    e2.reserved = 0;
    e2.pos_x = 16000;
    e2.pos_y = 10000;
    e2.angle = 90;
    packet.entities.push_back(e2);

    PacketBuffer buffer;
    packet.serialize(buffer, 5000);

    // 12 header + 4 payload header + (2 * 12 entity) = 40 bytes
    EXPECT_EQ(buffer.size(), 40);

    buffer.reset_read_offset();
    buffer.read_header();

    auto deserialized = WorldSnapshotPacket::deserialize(buffer);
    EXPECT_EQ(deserialized.entity_count, 2);
    ASSERT_EQ(deserialized.entities.size(), 2);
    EXPECT_EQ(deserialized.entities[0].entity_id.value, 100);
}

TEST(UDPPacketTest, EntityStateSize) {
    // RFC Section 6.2: EntityState MUST be 12 bytes
    EXPECT_EQ(sizeof(EntityState), 12);
}

TEST(UDPPacketTest, PlayerStatsPacketSize) {
    PlayerStatsPacket packet;
    packet.player_id = PlayerId{1};
    packet.lives = 3;
    packet.reserved = {0, 0};
    packet.score = 12345;

    PacketBuffer buffer;
    packet.serialize(buffer, 3000);

    // 12 byte header + 8 byte payload = 20 bytes
    EXPECT_EQ(buffer.size(), 20);
}

// ============================================================================
// PACKET FACTORY TESTS
// ============================================================================

TEST(PacketFactoryTest, DeserializeConnectReq) {
    ConnectReqPacket original;
    original.set_username("Player1");

    PacketBuffer buffer;
    original.serialize(buffer);

    auto result = deserialize_packet(buffer.data());
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(std::holds_alternative<ConnectReqPacket>(result.packet));

    auto &deserialized = std::get<ConnectReqPacket>(result.packet);
    EXPECT_EQ(deserialized.get_username(), "Player1");
    EXPECT_EQ(result.header.op_code, 0x01);
}

TEST(PacketFactoryTest, TooSmallPacket) {
    std::vector<uint8_t> data = {0x01};  // Only 1 byte

    auto result = deserialize_packet(data);
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("12 bytes"), std::string::npos);
}

TEST(PacketFactoryTest, UnknownOpCode) {
    PacketBuffer buffer;
    CommonHeader header(0xFF, 0);  // Invalid opcode
    buffer.write_header(header);

    auto result = deserialize_packet(buffer.data());
    EXPECT_FALSE(result.success);
}

TEST(PacketFactoryTest, SerializePacketVariant) {
    ConnectReqPacket packet;
    packet.set_username("TestUser");

    PacketVariant variant = packet;
    PacketBuffer buffer = serialize_packet(variant);

    EXPECT_EQ(buffer.size(), 44);  // 12 header + 32 payload

    auto result = deserialize_packet(buffer.data());
    EXPECT_TRUE(result.success);
}

// ============================================================================
// FRAGMENTATION TESTS
// ============================================================================

TEST(FragmentationTest, MultipleFragmentHeaders) {
    WorldSnapshotPacket packet1;
    packet1.entity_count = 0;
    packet1.reserved = {0, 0};

    PacketBuffer buffer1;
    packet1.serialize(buffer1, 1000, 0, 3);

    auto result1 = deserialize_packet(buffer1.data());
    EXPECT_TRUE(result1.success);
    EXPECT_EQ(result1.header.tick_id, 1000);
    EXPECT_EQ(result1.header.packet_index, 0);
    EXPECT_EQ(result1.header.packet_count, 3);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST(StressTest, ManyTCPPackets) {
    std::vector<PacketBuffer> buffers;

    for (int i = 0; i < 1000; ++i) {
        ConnectReqPacket packet;
        packet.set_username("Player" + std::to_string(i));

        PacketBuffer buffer;
        packet.serialize(buffer);
        buffers.push_back(std::move(buffer));
    }

    EXPECT_EQ(buffers.size(), 1000);

    for (size_t i = 0; i < buffers.size(); ++i) {
        auto result = deserialize_packet(buffers[i].data());
        ASSERT_TRUE(result.success);

        auto &packet = std::get<ConnectReqPacket>(result.packet);
        EXPECT_EQ(packet.get_username(), "Player" + std::to_string(i));
    }
}

TEST(StressTest, LargeSnapshot) {
    WorldSnapshotPacket packet;
    packet.entity_count = 100;
    packet.reserved = {0, 0};

    for (uint16_t i = 0; i < 100; ++i) {
        EntityState e;
        e.entity_id = EntityId{i};
        e.entity_type = i % 5;
        e.reserved = 0;
        e.pos_x = i * 100;
        e.pos_y = i * 50;
        e.angle = i * 3;
        packet.entities.push_back(e);
    }

    PacketBuffer buffer;
    packet.serialize(buffer, 50000);

    // 12 header + 4 payload header + (100 * 12) = 1216 bytes
    EXPECT_EQ(buffer.size(), 1216);

    auto result = deserialize_packet(buffer.data());
    ASSERT_TRUE(result.success);

    auto &deserialized = std::get<WorldSnapshotPacket>(result.packet);
    EXPECT_EQ(deserialized.entities.size(), 100);
}
