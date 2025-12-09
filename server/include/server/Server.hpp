#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include "include/registry.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"
#include "server/Packets.hpp"

namespace server {

/**
 * @brief Represents an active client connection to the server
 *
 * This structure encapsulates all state associated with a connected client,
 * including both TCP (reliable) and UDP (unreliable) endpoints, player
 * identification, and connection metadata.
 */
struct ClientConnection {
    uint8_t player_id_;
    boost::asio::ip::tcp::socket tcp_socket_;
    boost::asio::ip::udp::endpoint udp_endpoint_;
    std::chrono::steady_clock::time_point last_activity_;
    std::string username_;
    bool ready_;  // For future lobby system

    /**
     * @brief Construct a ClientConnection with moved socket
     *
     * @param id Assigned player ID (1-4)
     * @param socket TCP socket (ownership transferred)
     */
    ClientConnection(uint8_t id, boost::asio::ip::tcp::socket socket)
        : player_id_(id),
          tcp_socket_(std::move(socket)),
          udp_endpoint_(boost::asio::ip::udp::endpoint()),
          last_activity_(std::chrono::steady_clock::now()),
          ready_(false) {
        username_.reserve(32);  // Match CONNECT_REQ username size
    }

    // Disable copy (socket is not copyable)
    ClientConnection(const ClientConnection &) = delete;
    ClientConnection &operator=(const ClientConnection &) = delete;

    // Enable move
    ClientConnection(ClientConnection &&) = default;
    ClientConnection &operator=(ClientConnection &&) = default;
};

/**
 * @brief Main server class that manages game state using ECS
 *
 * This class integrates the Engine's ECS system with the server's
 * network and configuration components to manage the game state.
 *
 * Example usage:
 * @code
 * // Create and spawn entities with components
 * auto player = game_server.getRegistry().spawn_entity();
 * game_server.getRegistry().add_component(player,
 *     Component::Position{100.0f, 200.0f});
 * game_server.getRegistry().add_component(player,
 *     Component::Velocity{1.0f, 0.0f});
 * game_server.getRegistry().add_component(player,
 *     Component::Player{1, "Player1"});
 * @endcode
 */
class Server {
 public:
    /**
     * @brief Construct a new Server object
     *
     * @param config Server configuration
     * @param io_context Boost ASIO io_context for async operations
     */
    Server(Config &config, boost::asio::io_context &io_context);

    /**
     * @brief Destroy the Server object
     */
    ~Server();

    /**
     * @brief Initialize the server and register all components/systems
     */
    void Initialize();

    /**
     * @brief Start the server game loop
     */
    void Start();

    /**
     * @brief Update game state (called each tick)
     */
    void Update();

    /**
     * @brief Get the ECS registry
     *
     * @return Engine::registry& Reference to the registry
     */
    Engine::registry &GetRegistry();

 private:
    /**
     * @brief Register all ECS components
     */
    void RegisterComponents();

    /**
     * @brief Register all ECS systems
     */
    void RegisterSystems();

    /**
     * @brief Setup the game tick timer
     */
    void SetupGameTick();

    // ========================================================================
    // TCP Connection Handlers
    // ========================================================================

    /**
     * @brief Handle new TCP connection from acceptor
     *
     * Transfers socket ownership from Network layer to Server.
     * Initiates async read for CONNECT_REQ packet.
     *
     * @param socket Newly accepted TCP socket (ownership transferred)
     */
    void HandleTcpAccept(boost::asio::ip::tcp::socket socket);

    /**
     * @brief Process CONNECT_REQ packet and perform handshake
     *
     * Validates username, assigns PlayerId, sends CONNECT_ACK.
     *
     * @param socket TCP socket for this client
     * @param payload Deserialized CONNECT_REQ payload (32 bytes)
     */
    void HandleConnectReq(boost::asio::ip::tcp::socket &socket,
        const std::vector<uint8_t> &payload);

    /**
     * @brief Send CONNECT_ACK response to client
     *
     * @param socket TCP socket to write to
     * @param player_id Assigned player ID (0 if rejected)
     * @param status Connection status code (0=OK, 1=UsernameTaken,
     * 2=ServerFull)
     * @param socket_keeper Optional shared_ptr to keep socket alive for
     * rejected connections
     */
    void SendConnectAck(boost::asio::ip::tcp::socket &socket,
        uint8_t player_id, network::ConnectAckPacket::Status status,
        std::shared_ptr<boost::asio::ip::tcp::socket> socket_keeper = nullptr);

    /**
     * @brief Assign unique PlayerId to new client
     *
     * @return uint8_t PlayerId in range [1, 255]
     */
    uint8_t AssignPlayerId();

    /**
     * @brief Remove client from active connections
     *
     * Closes TCP socket and removes from clients_ map.
     *
     * @param player_id ID of client to remove
     */
    void RemoveClient(uint8_t player_id);

    /**
     * @brief Monitor client socket for disconnection
     *
     * Starts async read to detect when client closes connection.
     * Automatically removes client from clients_ map on disconnect.
     *
     * @param player_id ID of client to monitor
     */
    void MonitorClientDisconnect(uint8_t player_id);

    /**
     * @brief Check if username is already taken
     *
     * @param username Username to check
     * @return true if username exists in clients_
     */
    bool IsUsernameTaken(const std::string &username) const;

    // ========================================================================
    // Member Variables
    // ========================================================================

    Config &config_;
    boost::asio::io_context &io_context_;
    std::unique_ptr<Network> network_;
    Engine::registry registry_;
    boost::asio::steady_timer tick_timer_;
    bool running_;

    // Active client connections mapped by PlayerId
    std::unordered_map<uint8_t, ClientConnection> clients_;
    // Next available PlayerId
    uint8_t next_player_id_;

    static constexpr int TICK_RATE_MS = 16;    // ~60 FPS
    static constexpr uint8_t MAX_CLIENTS = 4;  // R-Type supports 4 players
};

}  // namespace server
