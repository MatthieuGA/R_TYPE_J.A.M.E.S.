#pragma once

#include <cstdint>

#include <boost/asio.hpp>

#include "server/ClientConnectionManager.hpp"
#include "server/Packets.hpp"

namespace server {

// Forward declaration
class Network;

/**
 * @brief Handles sending TCP packets to clients
 *
 * Responsible for:
 * - Serializing and sending TCP packets to individual clients
 * - Broadcasting packets to all authenticated players via TCP or UDP
 *
 * This class only sends data - no receiving or packet handling logic.
 */
class PacketSender {
 public:
    /**
     * @brief Construct a new PacketSender
     *
     * @param connection_manager Reference to the connection manager
     * @param network Reference to the network handler
     */
    explicit PacketSender(
        ClientConnectionManager &connection_manager, Network &network);

    /**
     * @brief Send CONNECT_ACK packet to client
     *
     * Helper function to send authentication response with server's UDP port.
     *
     * @param client Client connection to send to
     * @param status Response status (OK, ServerFull, BadUsername, InGame)
     * @param assigned_player_id Player ID if status is OK, 0 otherwise
     * @param udp_port Server's UDP port for client to send packets to
     */
    void SendConnectAck(ClientConnection &client,
        network::ConnectAckPacket::Status status,
        uint8_t assigned_player_id = 0, uint16_t udp_port = 0);

    /**
     * @brief Send GAME_START packet to all authenticated players
     *
     * RFC Section 5.5: Notifies all players that the game is starting.
     * Only sends to clients with player_id != 0 (authenticated).
     */
    void SendGameStart();

    void SendSnapshot(network::EntityState entity_state);

 private:
    // Reference to connection manager (not owned)
    ClientConnectionManager &connection_manager_;
    // Reference to network handler (not owned)
    Network &network_;
};

}  // namespace server
