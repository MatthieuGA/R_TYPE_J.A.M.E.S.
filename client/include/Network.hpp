#pragma once
/**
 * @brief Client-side network layer using Boost.Asio (TCP/UDP).
 *
 * Provides connection management, input sending, and snapshot reception.
 */
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>

// Server packet headers are available but we use a client-local type
// to avoid template instantiation issues with incomplete types.
#if __has_include(<server/Packets.hpp>) && __has_include(<server/PacketBuffer.hpp>)
#include <server/PacketBuffer.hpp>
#include <server/Packets.hpp>
#endif

namespace client {

/**
 * @brief Client-local snapshot packet.
 */
struct SnapshotPacket {
    uint32_t tick{0};
    std::vector<uint8_t> payload;
};

/**
 * @brief Client network manager handling TCP handshake and UDP gameplay.
 */
class Network {
 public:
    /**
     * @brief Construct a new Network instance.
     * @param io Boost.Asio io_context reference.
     * @param server_ip Server IPv4/IPv6 address or hostname.
     * @param tcp_port TCP control port.
     * @param udp_port UDP gameplay port.
     */
    Network(boost::asio::io_context &io, const std::string &server_ip,
        uint16_t tcp_port, uint16_t udp_port);

    /**
     * @brief Destructor. Closes sockets.
     */
    ~Network();

    /**
     * @brief Connect to server and send CONNECT_REQ.
     * @param username Player username (max 32 bytes, padded).
     */
    void ConnectToServer(const std::string &username);

    /**
     * @brief Send DISCONNECT_REQ and close sockets.
     */
    void Disconnect();

    /**
     * @brief Connection state.
     */
    bool IsConnected() const {
        return connected_;
    }

    /**
     * @brief Player identifier assigned by server.
     */
    uint8_t GetPlayerId() const {
        return player_id_;
    }

    /**
     * @brief Send input flags to server via UDP.
     * @param input_flags Bitfield of inputs.
     */
    void SendInput(uint8_t input_flags);

    /**
     * @brief Pop a world snapshot if available.
     */
    std::optional<client::SnapshotPacket> PollSnapshot();

 private:
    // Async handlers
    void AsyncReceiveUDP();
    void AsyncReceiveTCP();
    void HandleConnectAck(const std::vector<uint8_t> &data);

    // ASIO components
    boost::asio::io_context &io_context_;
    boost::asio::ip::udp::socket udp_socket_;
    boost::asio::ip::tcp::socket tcp_socket_;
    boost::asio::ip::udp::endpoint server_udp_endpoint_;

    // State
    bool connected_;
    uint8_t player_id_;
    uint32_t current_tick_;

    // Buffers
    std::array<uint8_t, 1472> udp_buffer_{};
    std::array<uint8_t, 512> tcp_buffer_{};

    // Snapshot queue (lock-free for async safety)
    boost::lockfree::spsc_queue<client::SnapshotPacket,
        boost::lockfree::capacity<256>>
        snapshot_queue_;

    // Connection params
    std::string server_ip_;
    uint16_t tcp_port_;
    uint16_t udp_port_;
};

}  // namespace client
