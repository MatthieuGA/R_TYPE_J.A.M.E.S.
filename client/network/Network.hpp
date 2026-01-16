#pragma once
/**
 * @brief Client-side network layer using Boost.Asio (TCP/UDP).
 *
 * Provides connection management, input sending, and snapshot reception.
 */
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace client {

/**
 * @brief Client-local snapshot packet.
 *
 * Uses fixed-size buffer to ensure trivial copyability for lock-free queue.
 * Max payload size is MTU (1500) - IP header (20) - UDP header (8) -
 * Our header (12) = 1460 bytes.
 */
struct SnapshotPacket {
    uint32_t tick{0};
    std::array<uint8_t, 1460> payload;
    uint16_t payload_size{0};
    uint8_t entity_type{0xFF};  // 0=Player, 1=Enemy, 2=Projectile, 0xFF=N/A
};

/**
 * @brief Client network manager handling TCP handshake and UDP gameplay.
 */
class ServerConnection {
 public:
    /**
     * @brief Construct a new Network instance.
     * @param io Boost.Asio io_context reference.
     * @param server_ip Server IPv4/IPv6 address or hostname.
     * @param tcp_port TCP control port.
     * @param udp_port UDP gameplay port.
     */
    ServerConnection(boost::asio::io_context &io, const std::string &server_ip,
        uint16_t tcp_port, uint16_t udp_port);

    /**
     * @brief Destructor. Closes sockets.
     */
    ~ServerConnection();

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
     * @brief Connection state accessor.
     * @return true if connected to server, false otherwise.
     */
    bool is_connected() const {
        return connected_.load();
    }

    /**
     * @brief Player identifier accessor.
     * @return Player ID assigned by server, or 0 if not connected.
     */
    uint8_t player_id() const {
        return player_id_.load();
    }

    /**
     * @brief Send input flags to server via UDP.
     * @param input_flags Bitfield of inputs.
     */
    void SendInput(uint8_t input_flags);

    /**
     * @brief Send ready status to server via TCP.
     * @param is_ready True if player is ready to start, false otherwise.
     */
    void SendReadyStatus(bool is_ready);

    /**
     * @brief Send game speed multiplier to server via TCP.
     * @param speed Game speed multiplier (0.25x to 2.0x).
     */
    void SendGameSpeed(float speed);

    /**
     * @brief Send killable enemy projectiles setting to server via TCP.
     * @param enabled Whether player projectiles can destroy enemy fire.
     * @todo(server-logic): Verify server logic applies this setting to game
     * mechanics. Check projectile collision detection system on server.
     */
    void SendKillableEnemyProjectiles(bool enabled);

    /**
     * @brief Send difficulty setting to server via TCP.
     * @param difficulty The difficulty level (Easy/Normal/Hard).
     * @todo(server-logic): Verify server logic applies damage multipliers
     * (0.75x for Easy, 1.0x for Normal, 1.1x for Hard) and enemy fire rate
     * multipliers (0.9x for Easy, 1.0x for Normal, 1.15x for Hard).
     */
    void SendDifficulty(uint8_t difficulty);

    /**
     * @brief Set callback for when game speed is changed by another player.
     * @param callback Function to call with the new speed value.
     */
    void SetOnGameSpeedChanged(std::function<void(float)> callback) {
        on_game_speed_changed_ = std::move(callback);
    }

    /**
     * @brief Set callback for when difficulty is changed by another player.
     * @param callback Function to call with the new difficulty value.
     */
    void SetOnDifficultyChanged(std::function<void(uint8_t)> callback) {
        on_difficulty_changed_ = std::move(callback);
    }

    /**
     * @brief Set callback for when killable projectiles is changed.
     * @param callback Function to call with the new enabled/disabled state.
     */
    void SetOnKillableProjectilesChanged(std::function<void(bool)> callback) {
        on_killable_projectiles_changed_ = std::move(callback);
    }

    /**
     * @brief Pop a world snapshot if available.
     */
    std::optional<client::SnapshotPacket> PollSnapshot();

    /**
     * @brief Check if game has started (received GAME_START packet).
     * @return true if GAME_START was received, false otherwise.
     */
    bool HasGameStarted() const {
        return game_started_.load();
    }

    /**
     * @brief Controlled entity id accessor (set by GAME_START)
     * @return Network entity id assigned to this client, or 0 if unknown
     */
    uint32_t controlled_entity_id() const {
        return controlled_entity_id_;
    }

    /**
     * @brief Reset the game started flag.
     * Useful after handling the GAME_START event.
     */
    void ResetGameStarted() {
        game_started_.store(false);
    }

    /**
     * @brief Get the number of connected players in the lobby.
     * @return Number of authenticated players (updated by LOBBY_STATUS)
     */
    uint8_t lobby_connected_count() const {
        return lobby_connected_count_.load();
    }

    /**
     * @brief Get the number of ready players in the lobby.
     * @return Number of ready players (updated by LOBBY_STATUS)
     */
    uint8_t lobby_ready_count() const {
        return lobby_ready_count_.load();
    }

    /**
     * @brief Get the maximum number of players allowed.
     * @return Maximum players (updated by LOBBY_STATUS)
     */
    uint8_t lobby_max_players() const {
        return lobby_max_players_.load();
    }

    /**
     * @brief Check if this player is ready.
     * @return true if local player has sent ready status, false otherwise.
     */
    bool is_local_player_ready() const {
        return is_local_player_ready_.load();
    }

    /**
     * @brief Check if game has ended (received GAME_END packet).
     * @return true if GAME_END was received, false otherwise.
     */
    bool HasGameEnded() const {
        return game_ended_.load();
    }

    /**
     * @brief Reset the game ended flag.
     * Useful after handling the GAME_END event.
     */
    void ResetGameEnded() {
        game_ended_.store(false);
        is_local_player_ready_.store(false);
        winning_player_id_.store(0);
    }

    /**
     * @brief Get the winning player ID from GAME_END packet.
     *
     * @return 0 = game over (all dead), 255 = victory (level complete),
     *         1-254 = specific player won
     */
    uint8_t GetWinningPlayerId() const {
        return winning_player_id_.load();
    }

    /**
     * @brief Check if the game ended in victory (level complete).
     * @return true if winning_player_id is 255 (level complete).
     */
    bool IsVictory() const {
        return game_ended_.load() && winning_player_id_.load() == 255;
    }

    /**
     * @brief Check if connection was rejected with a non-retryable status.
     * Status 3 (Game in Progress) should not be retried.
     * @return true if connection was rejected and should not retry.
     */
    bool WasRejectedPermanently() const {
        uint8_t status = last_rejection_status_.load();
        // Status 3 = Game in Progress (should not retry)
        return status == 3;
    }

    /**
     * @brief Get the last rejection status code.
     * @return 0 if no rejection, otherwise the status code from CONNECT_ACK.
     */
    uint8_t LastRejectionStatus() const {
        return last_rejection_status_.load();
    }

    /**
     * @brief Reset rejection status for a new connection attempt.
     */
    void ResetRejectionStatus() {
        last_rejection_status_.store(0);
    }

    /**
     * @brief Check if connection was lost unexpectedly.
     *
     * Returns true if the client was previously connected but the connection
     * was lost (server closed, network error, etc.). This is detected by
     * checking if connected_ became false after being true.
     *
     * @return true if connection was lost unexpectedly
     */
    bool WasDisconnectedUnexpectedly() const {
        return was_connected_once_.load() && !connected_.load();
    }

 private:
    // Async handlers
    void AsyncReceiveUDP();
    void AsyncReceiveTCP();
    void HandleConnectAck(const std::vector<uint8_t> &data);
    void HandleGameStart(const std::vector<uint8_t> &data);
    void HandleGameEnd(const std::vector<uint8_t> &data);
    void HandleNotifyDisconnect(const std::vector<uint8_t> &data);
    void HandleNotifyConnect(const std::vector<uint8_t> &data);
    void HandleNotifyReady(const std::vector<uint8_t> &data);
    void HandleNotifyGameSpeed(const std::vector<uint8_t> &data);
    void HandleNotifyDifficulty(const std::vector<uint8_t> &data);
    void HandleNotifyKillableProjectiles(const std::vector<uint8_t> &data);

    // Callback for game speed change notifications
    std::function<void(float)> on_game_speed_changed_;
    // Callbacks for difficulty and killable projectiles change notifications
    std::function<void(uint8_t)> on_difficulty_changed_;
    std::function<void(bool)> on_killable_projectiles_changed_;

    // ASIO components
    boost::asio::io_context &io_context_;
    boost::asio::ip::udp::socket udp_socket_;
    boost::asio::ip::tcp::socket tcp_socket_;
    boost::asio::ip::udp::endpoint server_udp_endpoint_;

    // State (atomic for thread-safe access from async handlers)
    std::atomic<bool> connected_;
    std::atomic<uint8_t> player_id_;
    std::atomic<bool> game_started_;
    std::atomic<bool> game_ended_{false};
    std::atomic<uint8_t> winning_player_id_{0};  ///< 0=gameover, 255=victory
    std::atomic<bool> was_connected_once_{
        false};  // Track if we ever connected
    uint32_t current_tick_;
    // Entity id the server told us we control (from GAME_START packet)
    uint32_t controlled_entity_id_{0};

    // Lobby status (updated by LOBBY_STATUS packet)
    std::atomic<uint8_t> lobby_connected_count_{0};
    std::atomic<uint8_t> lobby_ready_count_{0};
    std::atomic<uint8_t> lobby_max_players_{4};  // Default to 4
    std::atomic<bool> is_local_player_ready_{false};

    // Connection rejection status (0 = no rejection, >0 = status code)
    std::atomic<uint8_t> last_rejection_status_{0};

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
