#pragma once

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "server/Packets.hpp"

namespace server::network {

/**
 * @brief Variant type holding any possible packet
 *
 * Using std::variant ensures type safety and avoids heap allocation.
 * Each packet type is a distinct alternative in the variant.
 */
using PacketVariant = std::variant<
    // TCP Session Management
    ConnectReqPacket, ConnectAckPacket, DisconnectReqPacket,
    NotifyDisconnectPacket, GameStartPacket, GameEndPacket, ReadyStatusPacket,
    SetGameSpeedPacket, SetDifficultyPacket, SetKillableProjectilesPacket,
    // UDP Gameplay
    PlayerInputPacket, WorldSnapshotPacket, PlayerStatsPacket>;

/**
 * @brief Result of packet deserialization
 */
struct PacketParseResult {
    bool success;
    PacketVariant packet;
    CommonHeader header;  // Include parsed header for tick/fragmentation info
    std::string error;

    explicit operator bool() const {
        return success;
    }
};

/**
 * @brief Factory function to deserialize RFC-compliant packets
 *
 * @param data Raw byte data containing the packet
 * @param size Size of the data in bytes
 * @return PacketParseResult containing the parsed packet or error
 *
 * RFC Section 4.1: All packets begin with a 12-byte CommonHeader.
 * This function:
 * 1. Reads the 12-byte header
 * 2. Validates payload size matches buffer
 * 3. Dispatches to appropriate deserialize function based on OpCode
 * 4. Returns a variant containing the strongly-typed packet
 */
inline PacketParseResult DeserializePacket(const uint8_t *data, size_t size) {
    try {
        if (size < 12) {
            return PacketParseResult{false, ConnectReqPacket{}, CommonHeader{},
                "Packet too small: minimum 12 bytes (header) required"};
        }

        PacketBuffer buffer(data, size);
        CommonHeader header = buffer.ReadHeader();

        // Validate payload size
        if (buffer.Remaining() < header.payload_size) {
            return PacketParseResult{false, ConnectReqPacket{}, header,
                "Payload size mismatch: header claims " +
                    std::to_string(header.payload_size) + " bytes but only " +
                    std::to_string(buffer.Remaining()) + " available"};
        }

        PacketType packet_type = static_cast<PacketType>(header.op_code);

        switch (packet_type) {
            // TCP Session Management (0x01-0x07)
            case PacketType::ConnectReq:
                return PacketParseResult{
                    true, ConnectReqPacket::Deserialize(buffer), header, ""};

            case PacketType::ConnectAck:
                return PacketParseResult{
                    true, ConnectAckPacket::Deserialize(buffer), header, ""};

            case PacketType::DisconnectReq:
                return PacketParseResult{true,
                    DisconnectReqPacket::Deserialize(buffer), header, ""};

            case PacketType::NotifyDisconnect:
                return PacketParseResult{true,
                    NotifyDisconnectPacket::Deserialize(buffer), header, ""};

            case PacketType::GameStart:
                return PacketParseResult{
                    true, GameStartPacket::Deserialize(buffer), header, ""};

            case PacketType::GameEnd:
                return PacketParseResult{
                    true, GameEndPacket::Deserialize(buffer), header, ""};

            case PacketType::ReadyStatus:
                return PacketParseResult{
                    true, ReadyStatusPacket::Deserialize(buffer), header, ""};

            case PacketType::SetGameSpeed:
                return PacketParseResult{
                    true, SetGameSpeedPacket::Deserialize(buffer), header, ""};

            case PacketType::SetDifficulty:
                return PacketParseResult{
                    true, SetDifficultyPacket::Deserialize(buffer), header, ""};

            case PacketType::SetKillableProjectiles:
                return PacketParseResult{true,
                    SetKillableProjectilesPacket::Deserialize(buffer), header,
                    ""};

            // UDP Gameplay (0x10+)
            case PacketType::PlayerInput:
                return PacketParseResult{
                    true, PlayerInputPacket::Deserialize(buffer), header, ""};

            case PacketType::WorldSnapshot:
                return PacketParseResult{true,
                    WorldSnapshotPacket::Deserialize(buffer), header, ""};

            case PacketType::PlayerStats:
                return PacketParseResult{
                    true, PlayerStatsPacket::Deserialize(buffer), header, ""};

            default:
                return PacketParseResult{false, ConnectReqPacket{}, header,
                    "Unknown packet OpCode: 0x" +
                        std::to_string(static_cast<int>(header.op_code))};
        }
    } catch (const std::out_of_range &e) {
        return PacketParseResult{false, ConnectReqPacket{}, CommonHeader{},
            std::string("Buffer overflow: ") + e.what()};
    } catch (const std::exception &e) {
        return PacketParseResult{false, ConnectReqPacket{}, CommonHeader{},
            std::string("Deserialization error: ") + e.what()};
    }
}

/**
 * @brief Convenience overload for vector input
 */
inline PacketParseResult DeserializePacket(const std::vector<uint8_t> &data) {
    return DeserializePacket(data.data(), data.size());
}

/**
 * @brief Serialize any packet variant to a buffer
 *
 * @param packet The packet variant to serialize
 * @param tick_id The current tick ID (for UDP packets, 0 for TCP)
 * @param packet_index Fragment index (for large snapshots)
 * @param packet_count Total fragments for this tick
 * @return PacketBuffer containing the serialized data
 *
 * Note: TCP packets ignore tick_id, packet_index, packet_count parameters
 */
inline PacketBuffer SerializePacket(const PacketVariant &packet,
    uint32_t tick_id = 0, uint8_t packet_index = 0, uint8_t packet_count = 1) {
    PacketBuffer buffer;

    std::visit(
        [&](const auto &p) {
            using T = std::decay_t<decltype(p)>;

            // TCP packets (ignore tick parameters)
            constexpr bool is_tcp_packet =
                std::is_same_v<T, ConnectReqPacket> ||
                std::is_same_v<T, ConnectAckPacket> ||
                std::is_same_v<T, DisconnectReqPacket> ||
                std::is_same_v<T, NotifyDisconnectPacket> ||
                std::is_same_v<T, GameStartPacket> ||
                std::is_same_v<T, GameEndPacket> ||
                std::is_same_v<T, ReadyStatusPacket>;

            constexpr bool is_player_input_packet =
                std::is_same_v<T, PlayerInputPacket> ||
                std::is_same_v<T, PlayerStatsPacket>;

            constexpr bool is_world_snapshot_packet =
                std::is_same_v<T, WorldSnapshotPacket>;

            if constexpr (is_tcp_packet) {
                p.Serialize(buffer);
            } else if constexpr (is_player_input_packet) {
                p.Serialize(buffer, tick_id);
            } else if constexpr (is_world_snapshot_packet) {
                p.Serialize(buffer, tick_id, packet_index, packet_count);
            }
        },
        packet);

    return buffer;
}

}  // namespace server::network
