#include "network/Network.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace client {

namespace {
// Protocol constants
constexpr uint8_t kOpConnectReq = 0x01;
constexpr uint8_t kOpConnectAck = 0x02;
constexpr uint8_t kOpDisconnectReq = 0x03;
constexpr uint8_t kOpNotifyDisconnect = 0x04;
constexpr uint8_t kOpGameStart = 0x05;
constexpr uint8_t kOpGameEnd = 0x06;
constexpr uint8_t kOpReadyStatus = 0x07;
constexpr uint8_t kOpNotifyConnect = 0x08;
constexpr uint8_t kOpNotifyReady = 0x09;
constexpr uint8_t kOpSetGameSpeed = 0x0A;
constexpr uint8_t kOpNotifyGameSpeed = 0x0B;
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

ServerConnection::ServerConnection(boost::asio::io_context &io,
    const std::string &server_ip, uint16_t tcp_port, uint16_t udp_port)
    : io_context_(io),
      udp_socket_(io),
      tcp_socket_(io),
      connected_(false),
      player_id_(0),
      game_started_(false),
      current_tick_(0),
      server_ip_(server_ip),
      tcp_port_(tcp_port),
      udp_port_(udp_port) {
    try {
        // Bind to an auto-assigned port (0 = let OS choose)
        // The server will discover the actual port from the first UDP packet
        boost::asio::ip::udp::endpoint local_ep(boost::asio::ip::udp::v4(), 0);
        udp_socket_.open(boost::asio::ip::udp::v4());
        udp_socket_.bind(local_ep);

        // Display the auto-assigned port
        uint16_t bound_port = udp_socket_.local_endpoint().port();
        std::cout << "[Network] UDP socket bound to auto-assigned port "
                  << bound_port << std::endl;

        server_udp_endpoint_ = boost::asio::ip::udp::endpoint(
            boost::asio::ip::make_address(server_ip_), udp_port_);

        AsyncReceiveUDP();
    } catch (const std::exception &e) {
        std::cerr << "[Network] UDP init error: " << e.what() << std::endl;
    }
}

ServerConnection::~ServerConnection() {
    Disconnect();
}

void ServerConnection::ConnectToServer(const std::string &username) {
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
                std::cout << "[Network] TCP connection established"
                          << std::endl;

                // Build CONNECT_REQ packet: 12 byte header + 32 byte username
                auto pkt =
                    std::make_shared<std::vector<uint8_t>>(kHeaderSize + 32);
                WriteHeader(pkt->data(), kOpConnectReq, 32,
                    static_cast<uint32_t>(current_tick_));

                // Username padded/truncated to 32 bytes
                std::array<uint8_t, 32> uname{};
                std::memcpy(uname.data(), username.data(),
                    std::min<size_t>(username.size(), 32));
                std::memcpy(pkt->data() + kHeaderSize, uname.data(), 32);

                std::cout << "[Network] Sending CONNECT_REQ (" << pkt->size()
                          << " bytes, username: " << username << ")"
                          << std::endl;

                boost::asio::async_write(tcp_socket_,
                    boost::asio::buffer(*pkt),
                    [this, pkt](const boost::system::error_code &wec,
                        std::size_t bytes_sent) {
                        if (wec) {
                            std::cerr << "[Network] CONNECT_REQ send failed: "
                                      << wec.message() << std::endl;
                            return;
                        }
                        std::cout
                            << "[Network] CONNECT_REQ sent (" << bytes_sent
                            << " bytes). Waiting for ACK..." << std::endl;
                        AsyncReceiveTCP();
                    });
            });
    } catch (const std::exception &e) {
        std::cerr << "[Network] ConnectToServer error: " << e.what()
                  << std::endl;
    }
}

void ServerConnection::AsyncReceiveTCP() {
    // Read header
    boost::asio::async_read(tcp_socket_,
        boost::asio::buffer(tcp_buffer_.data(), kHeaderSize),
        [this](const boost::system::error_code &ec, std::size_t bytes_read) {
            if (ec) {
                if (ec == boost::asio::error::eof) {
                    std::cerr << "[Network] Server closed connection (EOF)"
                              << std::endl;
                    connected_.store(false);
                } else if (ec != boost::asio::error::operation_aborted) {
                    std::cerr
                        << "[Network] TCP header read error: " << ec.message()
                        << std::endl;
                    connected_.store(false);
                }
                return;
            }
            // Ensure we read the full header before accessing buffer fields
            if (bytes_read != kHeaderSize) {
                std::cerr << "[Network] TCP header incomplete: got "
                          << bytes_read << " bytes, expected " << kHeaderSize
                          << std::endl;
                return;
            }
            uint8_t opcode = tcp_buffer_[0];
            uint16_t payload_size = ReadLe16(tcp_buffer_.data() + 1);

            std::cout << "[Network] Received TCP packet: opcode=0x" << std::hex
                      << static_cast<int>(opcode) << std::dec
                      << ", payload_size=" << payload_size << std::endl;

            if (payload_size > tcp_buffer_.size() - kHeaderSize) {
                std::cerr << "[Network] TCP payload too large: "
                          << payload_size << " bytes (max: "
                          << (tcp_buffer_.size() - kHeaderSize) << ")"
                          << std::endl;
                Disconnect();
                return;
            }
            // Read payload
            boost::asio::async_read(tcp_socket_,
                boost::asio::buffer(
                    tcp_buffer_.data() + kHeaderSize, payload_size),
                [this, opcode, payload_size](
                    const boost::system::error_code &pec, std::size_t /*pn*/) {
                    if (pec) {
                        if (pec == boost::asio::error::eof) {
                            std::cerr
                                << "[Network] Server closed connection (EOF)"
                                << std::endl;
                            connected_.store(false);
                        } else {
                            std::cerr << "[Network] TCP payload read error: "
                                      << pec.message() << std::endl;
                            connected_.store(false);
                        }
                        return;
                    }
                    std::vector<uint8_t> data(payload_size);
                    std::memcpy(data.data(), tcp_buffer_.data() + kHeaderSize,
                        payload_size);
                    if (opcode == kOpConnectAck) {
                        HandleConnectAck(data);
                    } else if (opcode == kOpGameStart) {
                        HandleGameStart(data);
                    } else if (opcode == kOpGameEnd) {
                        HandleGameEnd(data);
                    } else if (opcode == kOpNotifyDisconnect) {
                        HandleNotifyDisconnect(data);
                    } else if (opcode == kOpNotifyDisconnect) {
                        HandleNotifyDisconnect(data);
                    } else if (opcode == kOpNotifyConnect) {
                        HandleNotifyConnect(data);
                    } else if (opcode == kOpNotifyReady) {
                        HandleNotifyReady(data);
                    } else if (opcode == kOpNotifyGameSpeed) {
                        HandleNotifyGameSpeed(data);
                    } else {
                        std::cout << "[Network] Unhandled TCP opcode: 0x"
                                  << std::hex << static_cast<int>(opcode)
                                  << std::dec << std::endl;
                    }
                    // Keep listening for next messages
                    AsyncReceiveTCP();
                });
        });
}

void ServerConnection::HandleConnectAck(const std::vector<uint8_t> &data) {
    const int kDebugLogLimit = 4;

    if (data.size() < kDebugLogLimit) {  // Payload is 8 bytes: PlayerId +
                                         // Status + ConnectedPlayers +
        // ReadyPlayers + MaxPlayers + MinPlayers + Reserved(u16)
        std::cerr << "[Network] CONNECT_ACK malformed (expected 8 bytes, got "
                  << data.size() << ")" << std::endl;
        return;
    }
    uint8_t pid = data[0];                          // PlayerId
    uint8_t status = data[1];                       // Status
    uint8_t connected = data[2];                    // ConnectedPlayers
    uint8_t ready = data[3];                        // ReadyPlayers
    uint8_t max_players = data[4];                  // MaxPlayers
    uint8_t min_players = data[5];                  // MinPlayers
    uint16_t reserved = ReadLe16(data.data() + 6);  // Reserved
    (void)min_players;                              // Unused for now
    (void)reserved;                                 // Unused

    if (status == 0x00) {
        player_id_.store(pid);
        connected_.store(true);
        was_connected_once_.store(true);  // Track that we connected

        // Initialize lobby counters from CONNECT_ACK
        lobby_connected_count_.store(connected);
        lobby_ready_count_.store(ready);
        lobby_max_players_.store(max_players);

        std::cout << "[Network] Connected. PlayerId=" << static_cast<int>(pid)
                  << ", Server reports: " << static_cast<int>(connected) << "/"
                  << static_cast<int>(max_players) << " players, "
                  << static_cast<int>(ready) << " ready" << std::endl;

        // Get the local UDP port assigned by OS and send discovery packet
        try {
            boost::asio::ip::udp::endpoint local_ep =
                udp_socket_.local_endpoint();
            uint16_t local_udp_port = local_ep.port();
            std::cout << "[Network] Local UDP port: " << local_udp_port
                      << std::endl;

            // Send a discovery packet to inform server of our UDP port
            // Format: opcode (0x10 = PLAYER_INPUT) + inputs + reserved
            // This also serves as endpoint discovery
            auto discovery_pkt =
                std::make_shared<std::vector<uint8_t>>(kHeaderSize + 4);
            WriteHeader(discovery_pkt->data(), 0x10, 4,
                static_cast<uint32_t>(current_tick_));
            // Include player id in discovery payload so server can map
            // this UDP endpoint to the correct authenticated client.
            (*discovery_pkt)[kHeaderSize + 0] = static_cast<uint8_t>(
                player_id_.load());                 // player id (discovery)
            (*discovery_pkt)[kHeaderSize + 1] = 0;  // reserved[0]
            (*discovery_pkt)[kHeaderSize + 2] = 0;  // reserved[1]
            (*discovery_pkt)[kHeaderSize + 3] = 0;  // reserved[2]

            udp_socket_.async_send_to(boost::asio::buffer(*discovery_pkt),
                server_udp_endpoint_,
                [this, discovery_pkt, local_udp_port](
                    const boost::system::error_code &ec, std::size_t) {
                    if (!ec) {
                        std::cout
                            << "[Network] UDP discovery packet sent from port "
                            << local_udp_port << std::endl;
                    } else {
                        std::cerr << "[Network] Failed to send UDP discovery: "
                                  << ec.message() << std::endl;
                    }
                });
        } catch (const std::exception &e) {
            std::cerr << "[Network] Error getting local UDP endpoint: "
                      << e.what() << std::endl;
        }
    } else {
        std::cerr << "[Network] CONNECT_ACK failed. Status="
                  << static_cast<int>(status) << std::endl;
        last_rejection_status_.store(status);
        Disconnect();
    }
}

void ServerConnection::HandleGameStart(const std::vector<uint8_t> &data) {
    if (data.size() < 4) {  // Payload is 4 bytes: ControlledEntityId (u32)
        std::cerr << "[Network] GAME_START malformed" << std::endl;
        return;
    }

    // Read controlled entity ID (little-endian u32)
    uint32_t controlled_entity_id = static_cast<uint32_t>(data[0]) |
                                    (static_cast<uint32_t>(data[1]) << 8) |
                                    (static_cast<uint32_t>(data[2]) << 16) |
                                    (static_cast<uint32_t>(data[3]) << 24);

    std::cout << "[Network] GAME_START received! Controlled EntityId="
              << controlled_entity_id << std::endl;

    // Store controlled entity id for local input mapping
    controlled_entity_id_ = controlled_entity_id;
    game_started_.store(true);
    // Reset game_ended flag so a new game doesn't immediately end
    game_ended_.store(false);
}

void ServerConnection::HandleGameEnd(const std::vector<uint8_t> &data) {
    if (data.size() < 4) {  // Payload is 4 bytes: winning_player_id + reserved
        std::cerr << "[Network] GAME_END malformed" << std::endl;
        return;
    }

    uint8_t winning_player_id = data[0];

    std::cout << "[Network] GAME_END received! WinningPlayerId="
              << static_cast<int>(winning_player_id) << std::endl;

    game_ended_.store(true);
    game_started_.store(false);
    lobby_ready_count_.store(0);  // Reset ready count for lobby display
    is_local_player_ready_.store(false);  // Reset local player's ready state
}

void ServerConnection::HandleNotifyDisconnect(
    const std::vector<uint8_t> &data) {
    // Payload: 4 bytes (PlayerId u8 + Reserved u8[3])
    if (data.size() < 4) {
        std::cerr << "[Network] NOTIFY_DISCONNECT malformed (size="
                  << data.size() << ", expected 4)" << std::endl;
        return;
    }

    uint8_t disconnected_player_id = data[0];

    // Don't update counter if this notification is about ourselves
    if (disconnected_player_id == player_id_.load()) {
        std::cout << "[Network] Ignoring NOTIFY_DISCONNECT about self"
                  << std::endl;
        return;
    }

    std::cout << "[Network] NOTIFY_DISCONNECT: Player "
              << static_cast<int>(disconnected_player_id) << " left the lobby"
              << std::endl;

    // Decrement connected player count
    lobby_connected_count_.fetch_sub(1, std::memory_order_relaxed);
}

void ServerConnection::HandleNotifyConnect(const std::vector<uint8_t> &data) {
    // Payload: 36 bytes (PlayerId u8 + Reserved u8[3] + Username char[32])
    if (data.size() < 36) {
        std::cerr << "[Network] NOTIFY_CONNECT malformed (size=" << data.size()
                  << ", expected 36)" << std::endl;
        return;
    }

    uint8_t new_player_id = data[0];
    // Reserved bytes at data[1], data[2], data[3]

    // Don't increment counter if this notification is about ourselves
    if (new_player_id == player_id_.load()) {
        std::cout << "[Network] Ignoring NOTIFY_CONNECT about self"
                  << std::endl;
        return;
    }

    // Extract username (null-terminated, max 32 bytes)
    std::string username;
    username.reserve(32);
    for (size_t i = 4; i < 36 && data[i] != '\0'; ++i) {
        username.push_back(static_cast<char>(data[i]));
    }

    std::cout << "[Network] NOTIFY_CONNECT: Player "
              << static_cast<int>(new_player_id) << " ('" << username
              << "') joined the lobby" << std::endl;

    // Increment connected player count
    lobby_connected_count_.fetch_add(1, std::memory_order_relaxed);
}

void ServerConnection::HandleNotifyReady(const std::vector<uint8_t> &data) {
    // Payload: 4 bytes (PlayerId u8 + IsReady u8 + Reserved u8[2])
    if (data.size() < 4) {
        std::cerr << "[Network] NOTIFY_READY malformed (size=" << data.size()
                  << ", expected 4)" << std::endl;
        return;
    }

    uint8_t ready_player_id = data[0];
    uint8_t is_ready = data[1];
    // Reserved bytes at data[2], data[3]

    std::cout << "[Network] NOTIFY_READY: Player "
              << static_cast<int>(ready_player_id) << " is now "
              << (is_ready ? "READY" : "NOT READY") << std::endl;

    // Update ready count based on status change
    if (is_ready) {
        lobby_ready_count_.fetch_add(1, std::memory_order_relaxed);
    } else {
        lobby_ready_count_.fetch_sub(1, std::memory_order_relaxed);
    }
}

void ServerConnection::HandleNotifyGameSpeed(
    const std::vector<uint8_t> &data) {
    // Payload: 4 bytes (float as little-endian IEEE 754)
    if (data.size() < 4) {
        std::cerr << "[Network] NOTIFY_GAME_SPEED malformed (size="
                  << data.size() << ", expected 4)" << std::endl;
        return;
    }

    // Read float from little-endian bytes
    uint32_t speed_bits = static_cast<uint32_t>(data[0]) |
                          (static_cast<uint32_t>(data[1]) << 8) |
                          (static_cast<uint32_t>(data[2]) << 16) |
                          (static_cast<uint32_t>(data[3]) << 24);
    float speed;
    std::memcpy(&speed, &speed_bits, sizeof(float));

    std::cout << "[Network] NOTIFY_GAME_SPEED: New speed = " << speed << "x"
              << std::endl;

    // Invoke callback if registered
    if (on_game_speed_changed_) {
        on_game_speed_changed_(speed);
    }
}

void ServerConnection::SendInput(uint8_t input_flags) {
    if (!connected_.load())
        return;
    // PLAYER_INPUT payload: 4 bytes (input_flags + 3 reserved bytes)
    auto pkt = std::make_shared<std::vector<uint8_t>>(kHeaderSize + 4);
    WriteHeader(
        pkt->data(), kOpPlayerInput, 4, static_cast<uint32_t>(current_tick_));
    (*pkt)[kHeaderSize + 0] = input_flags;
    (*pkt)[kHeaderSize + 1] = 0;  // reserved[0]
    (*pkt)[kHeaderSize + 2] = 0;  // reserved[1]
    (*pkt)[kHeaderSize + 3] = 0;  // reserved[2]
    udp_socket_.async_send_to(boost::asio::buffer(*pkt), server_udp_endpoint_,
        [pkt](const boost::system::error_code &ec, std::size_t) {
            if (ec) {
                std::cerr << "[Network] UDP send input error: " << ec.message()
                          << std::endl;
            }
        });
}

void ServerConnection::SendReadyStatus(bool is_ready) {
    if (!connected_.load()) {
        std::cerr << "[Network] Cannot send READY_STATUS: not connected"
                  << std::endl;
        return;
    }

    // Track local ready state
    is_local_player_ready_.store(is_ready);

    // READY_STATUS (0x07) payload: 4 bytes (IsReady + 3 reserved bytes)
    constexpr uint8_t kOpReadyStatus = 0x07;
    auto pkt = std::make_shared<std::vector<uint8_t>>(kHeaderSize + 4);
    WriteHeader(pkt->data(), kOpReadyStatus, 4, 0);  // TickId = 0 for TCP
    (*pkt)[kHeaderSize + 0] = is_ready ? 0x01 : 0x00;
    (*pkt)[kHeaderSize + 1] = 0;  // reserved[0]
    (*pkt)[kHeaderSize + 2] = 0;  // reserved[1]
    (*pkt)[kHeaderSize + 3] = 0;  // reserved[2]

    std::cout << "[Network] Sending READY_STATUS ("
              << (is_ready ? "Ready" : "Not Ready") << ")" << std::endl;

    boost::asio::async_write(tcp_socket_, boost::asio::buffer(*pkt),
        [pkt, is_ready](
            const boost::system::error_code &ec, std::size_t bytes_sent) {
            if (ec) {
                std::cerr << "[Network] Failed to send READY_STATUS: "
                          << ec.message() << std::endl;
            } else {
                std::cout << "[Network] READY_STATUS sent successfully ("
                          << bytes_sent << " bytes)" << std::endl;
            }
        });
}

void ServerConnection::SendGameSpeed(float speed) {
    if (!connected_.load()) {
        std::cerr << "[Network] Cannot send SET_GAME_SPEED: not connected"
                  << std::endl;
        return;
    }

    // SET_GAME_SPEED (0x0A) payload: 4 bytes (float)
    constexpr uint8_t kOpSetGameSpeed = 0x0A;
    auto pkt = std::make_shared<std::vector<uint8_t>>(kHeaderSize + 4);
    WriteHeader(pkt->data(), kOpSetGameSpeed, 4, 0);  // TickId = 0 for TCP

    // Write float as bytes (little-endian)
    uint32_t speed_bits;
    std::memcpy(&speed_bits, &speed, sizeof(float));
    (*pkt)[kHeaderSize + 0] = (speed_bits >> 0) & 0xFF;
    (*pkt)[kHeaderSize + 1] = (speed_bits >> 8) & 0xFF;
    (*pkt)[kHeaderSize + 2] = (speed_bits >> 16) & 0xFF;
    (*pkt)[kHeaderSize + 3] = (speed_bits >> 24) & 0xFF;

    std::cout << "[Network] Sending SET_GAME_SPEED (" << speed << "x)"
              << std::endl;

    boost::asio::async_write(tcp_socket_, boost::asio::buffer(*pkt),
        [pkt, speed](
            const boost::system::error_code &ec, std::size_t bytes_sent) {
            if (ec) {
                std::cerr << "[Network] Failed to send SET_GAME_SPEED: "
                          << ec.message() << std::endl;
            } else {
                std::cout << "[Network] SET_GAME_SPEED sent successfully ("
                          << bytes_sent << " bytes)" << std::endl;
            }
        });
}

void ServerConnection::AsyncReceiveUDP() {
    // Only schedule a new async receive if the socket is open.
    if (!udp_socket_.is_open())
        return;

    udp_socket_.async_receive_from(boost::asio::buffer(udp_buffer_),
        server_udp_endpoint_,
        [this](const boost::system::error_code &ec, std::size_t bytes) {
            if (ec) {
                if (ec != boost::asio::error::operation_aborted)
                    std::cerr
                        << "[Network] UDP receive error: " << ec.message()
                        << std::endl;
                return;
            }

            // Debug: log first 10 received packets with opcode
            static int udp_recv_count = 0;
            udp_recv_count++;

            if (bytes >= kHeaderSize) {
                uint8_t opcode = udp_buffer_[0];
                uint16_t payload_size = ReadLe16(udp_buffer_.data() + 1);
                uint8_t packet_index = udp_buffer_[3];
                uint32_t tick_id = ReadLe32(udp_buffer_.data() + 4);
                uint8_t packet_count = udp_buffer_[8];
                uint8_t entity_type =
                    udp_buffer_[9];  // Read entity_type from header

                if (udp_recv_count <= 10) {
                    std::cout << "[Network] UDP packet #" << udp_recv_count
                              << ": " << bytes << " bytes, opcode=0x"
                              << std::hex << static_cast<int>(opcode)
                              << std::dec << ", payload_size=" << payload_size
                              << ", tick=" << tick_id << ", entity_type="
                              << static_cast<int>(entity_type) << std::endl;
                }
                // Defensive check: Ensure payload_size doesn't exceed buffer
                // capacity (should never happen with proper MTU, but good
                // practice)
                if (payload_size > udp_buffer_.size() - kHeaderSize) {
                    std::cerr
                        << "[Network] UDP payload_size too large: "
                        << payload_size
                        << " (max: " << (udp_buffer_.size() - kHeaderSize)
                        << ")" << std::endl;
                    // Continue listening for next packet
                    if (udp_socket_.is_open())
                        AsyncReceiveUDP();
                    return;
                }
                if (opcode == kOpWorldSnapshot &&
                    bytes >= kHeaderSize + payload_size) {
                    client::SnapshotPacket snap;
                    snap.tick = tick_id;
                    snap.payload_size = payload_size;
                    snap.entity_type =
                        entity_type;  // Store entity_type in snapshot
                    std::memcpy(snap.payload.data(),
                        udp_buffer_.data() + kHeaderSize, payload_size);
                    if (!snapshot_queue_.push(snap)) {
                        std::cerr << "[Network] Snapshot queue full, "
                                  << "dropping packet" << std::endl;
                    } else if (udp_recv_count <= 10) {
                        std::cout << "[Network] Snapshot added to queue (tick="
                                  << tick_id << ")" << std::endl;
                    }
                } else if (udp_recv_count <= 10) {
                    std::cout << "[Network] Packet ignored: opcode=0x"
                              << std::hex << static_cast<int>(opcode)
                              << std::dec << ", expected=0x" << std::hex
                              << static_cast<int>(kOpWorldSnapshot) << std::dec
                              << std::endl;
                }
            }

            // Continue listening if still open
            if (udp_socket_.is_open())
                AsyncReceiveUDP();
        });
}

std::optional<client::SnapshotPacket> ServerConnection::PollSnapshot() {
    client::SnapshotPacket out;
    if (snapshot_queue_.pop(out)) {
        return out;
    }
    return std::nullopt;
}

void ServerConnection::Disconnect() {
    // DISCONNECT_REQ on TCP if connected
    try {
        if (tcp_socket_.is_open()) {
            std::array<uint8_t, kHeaderSize> pkt{};
            WriteHeader(pkt.data(), kOpDisconnectReq, 0,
                static_cast<uint32_t>(current_tick_));
            boost::system::error_code ec;
            (void)boost::asio::write(
                tcp_socket_, boost::asio::buffer(pkt), ec);
            // Cancel pending operations to wake up handlers.
            (void)tcp_socket_.cancel(ec);
        }
    } catch (...) {}
    boost::system::error_code ec;
    // Cancel UDP operations and close sockets. Cancel first so handlers
    // receive operation_aborted and can exit cleanly without being
    // rescheduled on closed sockets.
    if (udp_socket_.is_open()) {
        (void)udp_socket_.cancel(ec);
        (void)udp_socket_.close(ec);
    }
    if (tcp_socket_.is_open()) {
        (void)tcp_socket_.close(ec);
    }
    connected_.store(false);
}

}  // namespace client
