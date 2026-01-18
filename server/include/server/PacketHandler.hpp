#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/PacketFactory.hpp"
#include "server/PacketTypes.hpp"
#include "server/Packets.hpp"

namespace server {

// Forward declarations
class PacketSender;
class Network;

/**
 * @brief Handles receiving, dispatching and processing of incoming TCP packets
 *
 * Responsible for:
 * - Async TCP receive loop for each client
 * - Parsing incoming packets
 * - Detecting client disconnections
 * - Registering packet type handlers
 * - Routing incoming packets to appropriate handler functions
 * - Implementing game logic for each packet type (CONNECT_REQ, READY_STATUS,
 * etc.)
 *
 * This class contains the complete receive + process pipeline but delegates
 * connection management to ClientConnectionManager and network sends to
 * PacketSender.
 */
class PacketHandler {
 public:
    /**
     * @brief Type alias for packet handler functions
     *
     * Handlers receive a reference to the client connection and the parsed
     * packet.
     */
    using HandlerFunction = std::function<void(
        ClientConnection &client, const network::PacketVariant &packet)>;

    /**
     * @brief Type alias for game start callback
     */
    using GameStartCallback = std::function<void()>;

    /**
     * @brief Type alias for checking if game is running
     */
    using IsGameRunningCallback = std::function<bool()>;

    /**
     * @brief Construct a new PacketHandler
     *
     * @param connection_manager Reference to the connection manager
     * @param packet_sender Reference to the packet sender for responses
     * @param network Reference to network handler for UDP port info
     */
    PacketHandler(ClientConnectionManager &connection_manager,
        PacketSender &packet_sender, Network &network);

    /**
     * @brief Register all packet type handlers
     *
     * Maps PacketType enum values to handler functions.
     */
    void RegisterHandlers();

    /**
     * @brief Set callback for when all players are ready (game start)
     *
     * @param callback Function to call when game should start
     */
    void SetGameStartCallback(GameStartCallback callback);

    /**
     * @brief Set callback to check if game is running
     *
     * @param callback Function that returns true if game is in progress
     */
    void SetIsGameRunningCallback(IsGameRunningCallback callback);

    /**
     * @brief Start receiving messages from a client
     *
     * Initiates the async receive loop for the specified client.
     * Automatically handles disconnections and parses incoming packets.
     *
     * @param client_id Internal client ID
     */
    void StartReceiving(uint32_t client_id);

    /**
     * @brief Dispatch an incoming packet to the appropriate handler
     *
     * Looks up the handler for the packet's type and invokes it.
     * If no handler is registered, logs an error.
     *
     * @param client_id Internal client ID
     * @param result Parsed packet result from DeserializePacket
     */
    void Dispatch(
        uint32_t client_id, const network::PacketParseResult &result);

 private:
    /**
     * @brief Handle incoming TCP messages and monitor for disconnection
     *
     * Starts async read to receive TCP packets from the client.
     * Processes incoming messages and detects disconnections.
     * Automatically removes client from connection manager on disconnect.
     *
     * @param client_id Internal ID of client to handle
     */
    void HandleClientMessages(uint32_t client_id);

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

    /**
     * @brief Handle SET_GAME_SPEED packet from client
     *
     * Client sets game speed multiplier (0.25x to 2.0x).
     * Updates server's global game speed affecting all movement and timing.
     *
     * @param client Reference to client connection
     * @param packet Parsed SET_GAME_SPEED packet
     */
    void HandleSetGameSpeed(
        ClientConnection &client, const network::SetGameSpeedPacket &packet);

    /**
     * @brief Handle SET_DIFFICULTY packet
     * @param client Reference to client connection
     * @param packet Parsed SET_DIFFICULTY packet
     */
    void HandleSetDifficulty(
        ClientConnection &client, const network::SetDifficultyPacket &packet);

    /**
     * @brief Handle SET_KILLABLE_PROJECTILES packet
     * @param client Reference to client connection
     * @param packet Parsed SET_KILLABLE_PROJECTILES packet
     */
    void HandleSetKillableProjectiles(ClientConnection &client,
        const network::SetKillableProjectilesPacket &packet);

    /**
     * @brief Check if game should start after a player disconnect
     *
     * Called after removing a client to check if remaining players
     * in lobby are all ready. If so, starts the game.
     */
    void TryStartGameAfterDisconnect();

    /**
     * @brief Trim whitespace from string
     *
     * Helper function for username validation.
     *
     * @param str String to trim
     * @param whitespace Characters to remove (default: space and tab)
     * @return std::string Trimmed string
     */
    static std::string Trim(
        const std::string &str, const std::string &whitespace = " \t");

    // Packet type -> handler function mapping
    std::unordered_map<network::PacketType, HandlerFunction> packet_handlers_;

    // References to other components (not owned)
    ClientConnectionManager &connection_manager_;
    PacketSender &packet_sender_;
    Network &network_;

    // Callback for game start
    GameStartCallback on_game_start_;

    // Callback to check if game is running
    IsGameRunningCallback is_game_running_;
};

}  // namespace server
