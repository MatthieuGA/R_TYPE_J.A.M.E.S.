#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <boost/asio.hpp>

#include "include/registry.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"
#include "server/PacketFactory.hpp"
#include "server/PacketTypes.hpp"
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
    uint32_t client_id_;  // Internal unique ID (never sent over network)
    uint8_t player_id_;   // Network player ID (1-255, 0 if not authenticated)
    boost::asio::ip::tcp::socket tcp_socket_;
    boost::asio::ip::udp::endpoint udp_endpoint_;
    std::chrono::steady_clock::time_point last_activity_;
    std::string username_;
    bool ready_;  // For lobby ready state

    /**
     * @brief Construct a ClientConnection with moved socket
     *
     * @param cid Unique internal client ID
     * @param pid Assigned player ID (1-255, 0 if not yet authenticated)
     * @param socket TCP socket (ownership transferred)
     */
    ClientConnection(
        uint32_t cid, uint8_t pid, boost::asio::ip::tcp::socket socket)
        : client_id_(cid),
          player_id_(pid),
          tcp_socket_(std::move(socket)),
          udp_endpoint_(boost::asio::ip::udp::endpoint()),
          last_activity_(std::chrono::steady_clock::now()),
          ready_(false) {
        username_.reserve(32);  // Match CONNECT_REQ username size
    }

    /**
     * @brief Check if client is authenticated
     *
     * @return true if player_id is assigned (non-zero)
     */
    bool IsAuthenticated() const {
        return player_id_ != 0;
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
     * @brief Stop the game loop and reset game state and tick
     */
    void Stop();

    /**
     * @brief Stop the game loop and close all client connections
     */
    void Close();

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
     * Accepts incoming TCP connections and assigns a unique client_id.
     * Client starts with player_id=0 (unauthenticated) and must send
     * CONNECT_REQ to get a player_id (1-255) and complete authentication.
     *
     * @param socket Newly accepted TCP socket (ownership transferred)
     */
    void HandleTcpAccept(boost::asio::ip::tcp::socket socket);

    /**
     * @brief Assign unique internal client ID
     *
     * @return uint32_t Unique client ID (never reused)
     */
    uint32_t AssignClientId();

    /**
     * @brief Assign unique PlayerId to authenticated client
     *
     * @return uint8_t PlayerId in range [1, 255]
     */
    uint8_t AssignPlayerId();

    /**
     * @brief Remove client from active connections
     *
     * Closes TCP socket and removes from clients_ map.
     *
     * @param client_id Internal ID of client to remove
     */
    void RemoveClient(uint32_t client_id);

    /**
     * @brief Handle incoming TCP messages and monitor for disconnection
     *
     * Starts async read to receive TCP packets from the client.
     * Processes incoming messages and detects disconnections.
     * Automatically removes client from clients_ map on disconnect.
     *
     * @param client_id Internal ID of client to handle
     */
    void HandleClientMessages(uint32_t client_id);

    /**
     * @brief Check if username is already taken
     *
     * @param username Username to check
     * @return true if username exists in clients_
     */
    bool IsUsernameTaken(const std::string &username) const;

    // ========================================================================
    // TCP Packet Handlers
    // ========================================================================

    /**
     * @brief Type alias for packet handler functions
     */
    using PacketHandler = std::function<void(
        ClientConnection &client, const network::PacketVariant &packet)>;

    /**
     * @brief Register all TCP packet handlers
     *
     * Maps PacketType to handler functions for dispatch.
     */
    void RegisterPacketHandlers();

    /**
     * @brief Handle CONNECT_REQ packet (authentication)
     *
     * RFC Section 5.1: Client requests to join with username.
     * Validates username, assigns player_id (1-255), sends CONNECT_ACK.
     * Client can retry if rejected (e.g., bad username, server full).
     *
     * @param client Reference to client connection (player_id_ will be set)
     * @param packet Parsed CONNECT_REQ packet
     */
    void HandleConnectReq(
        ClientConnection &client, const network::ConnectReqPacket &packet);

    /**
     * @brief Handle READY_STATUS packet from client
     *
     * RFC Section 5.7: Client indicates ready state in lobby.
     * Sets ready_ flag in ClientConnection.
     *
     * @param client Reference to client connection
     * @param packet Parsed READY_STATUS packet
     */
    void HandleReadyStatus(
        ClientConnection &client, const network::ReadyStatusPacket &packet);

    /**
     * @brief Handle DISCONNECT_REQ packet from client
     *
     * RFC Section 5.3: Client requests graceful disconnection.
     * Removes client from active connections.
     *
     * @param client Reference to client connection
     * @param packet Parsed DISCONNECT_REQ packet
     */
    void HandleDisconnectReq(
        ClientConnection &client, const network::DisconnectReqPacket &packet);

    // ========================================================================
    // Member Variables
    // ========================================================================

    Config &config_;
    boost::asio::io_context &io_context_;
    std::unique_ptr<Network> network_;
    Engine::registry registry_;
    boost::asio::steady_timer tick_timer_;
    bool running_;

    // All client connections mapped by internal client_id
    // Use authenticated_ flag to distinguish authenticated vs pending
    std::unordered_map<uint32_t, ClientConnection> clients_;
    // Next available client ID (internal, never sent over network)
    uint32_t next_client_id_;
    // Next available PlayerId (1-255, sent in packets)
    uint8_t next_player_id_;

    // Packet type -> handler function mapping
    std::unordered_map<network::PacketType, PacketHandler> packet_handlers_;

    static constexpr int TICK_RATE_MS = 16;  // ~60 FPS
    uint8_t max_clients_;                    // Configured max players
};

}  // namespace server
