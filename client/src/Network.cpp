#include "Network.hpp"

#include <cstring>
#include <iomanip>
#include <iostream>

namespace client {

namespace {
// Protocol constants
constexpr uint8_t kOpConnectReq = 0x01;
constexpr uint8_t kOpConnectAck = 0x02;
constexpr uint8_t kOpDisconnectReq = 0x03;
constexpr uint8_t kOpPlayerInput = 0x10;
constexpr uint8_t kOpWorldSnapshot = 0x20;

// RFC-compliant 12-byte header: [opcode(1), payload_size(2), packet_index(1),
// tick_id(4), packet_count(1), reserved(3)]
constexpr size_t kHeaderSize = 12;

inline void WriteLe16(uint8_t *dst, uint16_t v) {
    dst[0] = static_cast<uint8_t>(v & 0xFF);
    dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

inline void WriteLe32(uint8_t *dst, uint32_t v) {
    for (int i = 0; i < 4; ++i)
        dst[i] = static_cast<uint8_t>((v >> (i * 8)) & 0xFF);
}

inline uint16_t ReadLe16(const uint8_t *src) {
    return static_cast<uint16_t>(src[0]) |
           (static_cast<uint16_t>(src[1]) << 8);
}

inline uint32_t ReadLe32(const uint8_t *src) {
    return static_cast<uint32_t>(src[0]) |
           (static_cast<uint32_t>(src[1]) << 8) |
           (static_cast<uint32_t>(src[2]) << 16) |
           (static_cast<uint32_t>(src[3]) << 24);
}

// Helper to write CommonHeader matching server/PacketBuffer.hpp layout
inline void WriteHeader(uint8_t *dst, uint8_t opcode, uint16_t payload_size,
    uint32_t tick_id, uint8_t packet_index = 0, uint8_t packet_count = 1) {
    dst[0] = opcode;
    WriteLe16(dst + 1, payload_size);
    dst[3] = packet_index;
    WriteLe32(dst + 4, tick_id);
    dst[8] = packet_count;
    dst[9] = 0;   // reserved[0]
    dst[10] = 0;  // reserved[1]
    dst[11] = 0;  // reserved[2]
}
}  // namespace

Network::Network(boost::asio::io_context &io, const std::string &server_ip,
    uint16_t tcp_port, uint16_t udp_port)
    : io_context_(io),
      udp_socket_(io),
      tcp_socket_(io),
      connected_(false),
      player_id_(0),
      current_tick_(0),
      server_ip_(server_ip),
      tcp_port_(tcp_port),
      udp_port_(udp_port) {
    try {
        boost::asio::ip::udp::endpoint local_ep(boost::asio::ip::udp::v4(), 0);
        udp_socket_.open(boost::asio::ip::udp::v4());
        udp_socket_.bind(local_ep);

        server_udp_endpoint_ = boost::asio::ip::udp::endpoint(
            boost::asio::ip::make_address(server_ip_), udp_port_);

        AsyncReceiveUDP();
    } catch (const std::exception &e) {
        std::cerr << "[Network] UDP init error: " << e.what() << std::endl;
    }
}

Network::~Network() {
    Disconnect();
}

void Network::ConnectToServer(const std::string &username) {
    using boost::asio::ip::tcp;
    try {
        tcp::endpoint server_ep(
            boost::asio::ip::make_address(server_ip_), tcp_port_);
        tcp_socket_.async_connect(
            server_ep, [this, username](const boost::system::error_code &ec) {
                if (ec) {
                    std::cerr
                        << "[Network] TCP connect failed: " << ec.message()
                        << std::endl;
                    return;
                }
                // Build CONNECT_REQ packet
                std::array<uint8_t, kHeaderSize + 32> pkt{};
                WriteHeader(pkt.data(), kOpConnectReq, 32,
                    static_cast<uint32_t>(current_tick_));
                // Username padded/truncated to 32 bytes
                std::array<uint8_t, 32> uname{};
                std::memcpy(uname.data(), username.data(),
                    std::min<size_t>(username.size(), 32));
                std::memcpy(pkt.data() + kHeaderSize, uname.data(), 32);

                boost::asio::async_write(tcp_socket_, boost::asio::buffer(pkt),
                    [this](const boost::system::error_code &wec, std::size_t) {
                        if (wec) {
                            std::cerr << "[Network] CONNECT_REQ send failed: "
                                      << wec.message() << std::endl;
                            return;
                        }
                        AsyncReceiveTCP();
                    });
            });
    } catch (const std::exception &e) {
        std::cerr << "[Network] ConnectToServer error: " << e.what()
                  << std::endl;
    }
}

void Network::AsyncReceiveTCP() {
    auto self = this;
    // Read header
    boost::asio::async_read(tcp_socket_,
        boost::asio::buffer(tcp_buffer_.data(), kHeaderSize),
        [this, self](const boost::system::error_code &ec, std::size_t /*n*/) {
            if (ec) {
                if (ec != boost::asio::error::eof)
                    std::cerr
                        << "[Network] TCP header read error: " << ec.message()
                        << std::endl;
                return;
            }
            uint8_t opcode = tcp_buffer_[0];
            uint16_t payload_size = ReadLe16(tcp_buffer_.data() + 1);
            if (payload_size > tcp_buffer_.size() - kHeaderSize) {
                std::cerr << "[Network] TCP payload too large: "
                          << payload_size << std::endl;
                return;
            }
            // Read payload
            boost::asio::async_read(tcp_socket_,
                boost::asio::buffer(
                    tcp_buffer_.data() + kHeaderSize, payload_size),
                [this, opcode, payload_size](
                    const boost::system::error_code &pec, std::size_t /*pn*/) {
                    if (pec) {
                        std::cerr << "[Network] TCP payload read error: "
                                  << pec.message() << std::endl;
                        return;
                    }
                    std::vector<uint8_t> data(payload_size);
                    std::memcpy(data.data(), tcp_buffer_.data() + kHeaderSize,
                        payload_size);
                    if (opcode == kOpConnectAck) {
                        HandleConnectAck(data);
                    } else {
                        // Unknown opcode on TCP in Phase 0, ignore
                    }
                    // Keep listening for next messages
                    AsyncReceiveTCP();
                });
        });
}

void Network::HandleConnectAck(const std::vector<uint8_t> &data) {
    if (data.size() <
        4) {  // Payload is 4 bytes: PlayerId + Status + Reserved[2]
        std::cerr << "[Network] CONNECT_ACK malformed" << std::endl;
        return;
    }
    uint8_t pid = data[0];     // PlayerId is first
    uint8_t status = data[1];  // Status is second
    if (status == 0x00) {
        connected_ = true;
        player_id_ = pid;
        std::cout << "[Network] Connected. PlayerId="
                  << static_cast<int>(player_id_) << std::endl;
    } else {
        std::cerr << "[Network] CONNECT_ACK failed. Status="
                  << static_cast<int>(status) << std::endl;
        Disconnect();
    }
}

void Network::SendInput(uint8_t input_flags) {
    if (!connected_)
        return;
    // PLAYER_INPUT payload: 4 bytes (input_flags + 3 reserved bytes)
    std::array<uint8_t, kHeaderSize + 4> pkt{};
    WriteHeader(
        pkt.data(), kOpPlayerInput, 4, static_cast<uint32_t>(current_tick_));
    pkt[kHeaderSize + 0] = input_flags;
    pkt[kHeaderSize + 1] = 0;  // reserved[0]
    pkt[kHeaderSize + 2] = 0;  // reserved[1]
    pkt[kHeaderSize + 3] = 0;  // reserved[2]
    udp_socket_.async_send_to(boost::asio::buffer(pkt), server_udp_endpoint_,
        [](const boost::system::error_code &ec, std::size_t) {
            if (ec) {
                std::cerr << "[Network] UDP send input error: " << ec.message()
                          << std::endl;
            }
        });
}

void Network::AsyncReceiveUDP() {
    udp_socket_.async_receive_from(boost::asio::buffer(udp_buffer_),
        server_udp_endpoint_,
        [this](const boost::system::error_code &ec, std::size_t bytes) {
            if (ec) {
                if (ec != boost::asio::error::operation_aborted)
                    std::cerr
                        << "[Network] UDP receive error: " << ec.message()
                        << std::endl;
            } else if (bytes >= kHeaderSize) {
                uint8_t opcode = udp_buffer_[0];
                uint16_t payload_size = ReadLe16(udp_buffer_.data() + 1);
                uint32_t tick_id = ReadLe32(udp_buffer_.data() + 4);
                if (opcode == kOpWorldSnapshot &&
                    bytes >= kHeaderSize + payload_size) {
                    client::SnapshotPacket snap;
                    snap.tick = tick_id;
                    snap.payload.assign(udp_buffer_.data() + kHeaderSize,
                        udp_buffer_.data() + kHeaderSize + payload_size);
                    snapshot_queue_.push(snap);
                }
            }
            // Continue listening
            AsyncReceiveUDP();
        });
}

std::optional<client::SnapshotPacket> Network::PollSnapshot() {
    client::SnapshotPacket out;
    if (snapshot_queue_.pop(out)) {
        return out;
    }
    return std::nullopt;
}

void Network::Disconnect() {
    // Best-effort DISCONNECT_REQ on TCP if connected
    try {
        if (tcp_socket_.is_open()) {
            std::array<uint8_t, kHeaderSize> pkt{};
            WriteHeader(pkt.data(), kOpDisconnectReq, 0,
                static_cast<uint32_t>(current_tick_));
            boost::system::error_code ec;
            boost::asio::write(tcp_socket_, boost::asio::buffer(pkt), ec);
        }
    } catch (...) {}
    boost::system::error_code ec;
    if (udp_socket_.is_open())
        udp_socket_.close(ec);
    if (tcp_socket_.is_open())
        tcp_socket_.close(ec);
    connected_ = false;
}

}  // namespace client
