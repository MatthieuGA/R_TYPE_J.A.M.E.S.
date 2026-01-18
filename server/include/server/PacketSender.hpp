#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

    /**
     * @brief Send GAME_END packet to all authenticated players with
     * leaderboard
     *
     * RFC Section 5.6: Notifies all players that the game has ended.
     * Includes leaderboard data with player names, scores, and rankings.
     *
     * @param winning_player_id The player who won (0 = all lost, 255 = level
     * complete)
     * @param game_mode 0 = finite, 1 = infinite
     * @param leaderboard Vector of player score data for display
     */
    void SendGameEnd(uint8_t winning_player_id, uint8_t game_mode,
        const std::vector<network::PlayerScoreData> &leaderboard);

    /**
     * @brief Send NOTIFY_CONNECT packet to all authenticated players
     *
     * RFC Section 5.8: Broadcasts when a new player joins the lobby.
     * Updates lobby UI on all clients with new player information.
     *
     * @param player_id The ID of the newly connected player
     * @param username The username of the newly connected player
     */
    void SendNotifyConnect(uint8_t player_id, const std::string &username);

    /**
     * @brief Send NOTIFY_READY packet to all authenticated players
     *
     * RFC Section 5.9: Broadcasts when a player changes ready status.
     * Updates lobby UI on all clients with player's ready state.
     *
     * @param player_id The ID of the player who changed status
     * @param is_ready True if player is ready, false otherwise
     */
    void SendNotifyReady(uint8_t player_id, bool is_ready);

    /**
     * @brief Send NOTIFY_DISCONNECT packet to all authenticated players
     *
     * RFC Section 5.4: Broadcasts when a player disconnects.
     * Updates lobby UI on all clients to remove the disconnected player.
     *
     * @param player_id The ID of the player who disconnected
     */
    void SendNotifyDisconnect(uint8_t player_id);

    void SendSnapshot(network::EntityState entity_state, int tick);

 private:
    // Reference to connection manager (not owned)
    ClientConnectionManager &connection_manager_;
    // Reference to network handler (not owned)
    Network &network_;
};

}  // namespace server
