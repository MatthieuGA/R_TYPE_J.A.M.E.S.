#pragma once

#include <cstdint>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/Packets.hpp"

namespace server {

// Forward declaration
class PacketDispatcher;

/**
 * @brief Handles all network communication with clients
 *
 * Responsible for:
 * - Receiving TCP messages asynchronously (HandleClientMessages loop)
 * - Sending TCP packets to clients (CONNECT_ACK, GAME_START, etc.)
 * - Managing async I/O operations
 *
 * This class owns the network I/O logic but does NOT handle packet parsing
 * or business logic - it delegates to PacketDispatcher for that.
 */
class ServerMessenger {
 public:
    /**
     * @brief Construct a new ServerMessenger
     *
     * @param connection_manager Reference to the connection manager
     */
    explicit ServerMessenger(ClientConnectionManager &connection_manager);

    /**
     * @brief Set the packet dispatcher for handling received packets
     *
     * Must be called before StartReceiving() to enable packet processing.
     *
     * @param dispatcher Pointer to the packet dispatcher (not owned)
     */
    void SetDispatcher(PacketDispatcher *dispatcher);

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
     * @brief Send CONNECT_ACK packet to client
     *
     * Helper function to send authentication response.
     *
     * @param client Client connection to send to
     * @param status Response status (OK, ServerFull, BadUsername, InGame)
     * @param assigned_player_id Player ID if status is OK, 0 otherwise
     */
    void SendConnectAck(ClientConnection &client,
        network::ConnectAckPacket::Status status,
        uint8_t assigned_player_id = 0);

    /**
     * @brief Send GAME_START packet to all authenticated players
     *
     * RFC Section 5.5: Notifies all players that the game is starting.
     * Only sends to clients with player_id != 0 (authenticated).
     */
    void SendGameStart();

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

    // Reference to connection manager (not owned)
    ClientConnectionManager &connection_manager_;

    // Pointer to packet dispatcher (not owned, set via SetDispatcher)
    PacketDispatcher *dispatcher_;
};

}  // namespace server
