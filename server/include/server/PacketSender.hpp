#pragma once

#include <cstdint>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/Packets.hpp"

namespace server {

/**
 * @brief Handles sending TCP packets to clients
 *
 * Responsible for:
 * - Serializing and sending TCP packets to individual clients
 * - Broadcasting packets to all authenticated players
 *
 * This class only sends data - no receiving or packet handling logic.
 */
class PacketSender {
 public:
    /**
     * @brief Construct a new PacketSender
     *
     * @param connection_manager Reference to the connection manager
     */
    explicit PacketSender(ClientConnectionManager &connection_manager);

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
    // Reference to connection manager (not owned)
    ClientConnectionManager &connection_manager_;
};

}  // namespace server
