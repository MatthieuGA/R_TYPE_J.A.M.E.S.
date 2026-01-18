#pragma once
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include "include/registry.hpp"
#include "server/ClientConnectionManager.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"
#include "server/PacketHandler.hpp"
#include "server/PacketSender.hpp"
#include "server/Packets.hpp"
#include "server/worldgen/WorldGen.hpp"

namespace server {

// Forward declarations
class WorldGenSystem;

/**
 * @brief Main server class that manages game state using ECS
 *
 * This class integrates the Engine's ECS system with the server's
 * network and configuration components to manage the game state.
 * It delegates connection management, packet handling, and messaging
 * to specialized manager classes.
 *
 * Example usage:
 * @code
 * // Create and spawn entities with components
 * auto player = game_server.GetRegistry().spawn_entity();
 * game_server.GetRegistry().add_component(player,
 *     Component::Position{100.0f, 200.0f});
 * game_server.GetRegistry().add_component(player,
 *     Component::Velocity{1.0f, 0.0f});
 * game_server.GetRegistry().add_component(player,
 *     Component::Player{1, "Player1"});
 * @endcode
 */
class Server {
 public:
    /**
     * @brief Get the singleton instance (for systems to notify player death)
     */
    static Server *GetInstance() {
        return instance_;
    }

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
     * @brief Display available levels and prompt user for selection.
     *
     * Lists all available levels with their names and finite/infinite status.
     * User selects by entering a number.
     */
    void PromptLevelSelection();

    /**
     * @brief Set the selected level UUID.
     *
     * @param level_uuid UUID of the level to play, empty for endless mode.
     */
    void SetSelectedLevel(const std::string &level_uuid);

    /**
     * @brief Get list of available levels for display.
     *
     * @return Vector of pairs: (level name, is_endless flag)
     */
    std::vector<std::pair<std::string, bool>> GetAvailableLevels() const;

    /**
     * @brief Start the server game loop
     */
    void Start();

    /**
     * @brief Stop the game loop and reset game state and tick
     */
    void Stop();

    /**
     * @brief Reset the server to lobby state after game over
     *
     * Clears all game entities, resets player ready states, and prepares
     * for a new game. Called when all players die (game over).
     */
    void ResetToLobby();

    /**
     * @brief Check if all player entities are dead (health <= 0)
     *
     * @return true if all players are dead or no players exist
     */
    bool AreAllPlayersDead();

    /**
     * @brief Notify the server that a player has died
     *
     * Called by HealthDeductionSystem when a player's health reaches 0.
     * Now also tracks player scores and death order for leaderboard.
     *
     * @param player_id The player ID that died (if known), 0 for auto-detect
     * @param final_score The player's final score at death
     */
    void NotifyPlayerDeath(uint8_t player_id = 0, int final_score = 0);

    /**
     * @brief Destroy the player entity associated with a player_id
     *
     * Searches for the player entity with matching playerNumber and kills it.
     * Called when a client disconnects during an active game.
     *
     * @param player_id The player ID to find and destroy
     * @return true if the entity was found and destroyed, false otherwise
     */
    bool DestroyPlayerEntity(uint8_t player_id);

    /**
     * @brief Handle player disconnect during active game
     *
     * Destroys the player's entity, updates tracking, and checks if game
     * should end (all players gone).
     *
     * @param player_id The player ID that disconnected
     */
    void HandlePlayerDisconnect(uint8_t player_id);

    /**
     * @brief Check if game is currently running
     * @return true if game is in progress
     */
    bool IsGameRunning() const {
        return running_;
    }

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

    static uint32_t GetNextNetworkId();

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

    void SendSnapshotsToAllClients();

    void SetupEntitiesGame();

    /**
     * @brief Handle new TCP connection from acceptor
     *
     * Accepts incoming TCP connections and adds them to connection manager.
     *
     * @param socket Newly accepted TCP socket (ownership transferred)
     */
    void HandleTcpAccept(boost::asio::ip::tcp::socket socket);

    /**
     * @brief Handle UDP PLAYER_INPUT packets (discovery or input bitfield)
     *
     * @param endpoint UDP endpoint the packet came from
     * @param data Raw packet bytes
     * @return true if the packet was handled (discovery or input applied)
     */
    bool HandleUdpPlayerInput(const boost::asio::ip::udp::endpoint &endpoint,
        const std::vector<uint8_t> &data);

    /**
     * @brief Determine the winner based on game rules.
     *
     * For finite mode with victory: highest score among survivors wins.
     * For infinite mode or all dead: whoever survived longest wins,
     * tie-break by score.
     *
     * @param is_victory True if level was completed, false if all died.
     * @return Player ID of the winner, 0 if no winner.
     */
    uint8_t DetermineWinner(bool is_victory);

    /**
     * @brief Build leaderboard data for GAME_END packet.
     *
     * @param is_victory True if level was completed.
     * @param winner_id The player ID that won.
     * @return Vector of PlayerScoreData sorted by ranking.
     */
    std::vector<network::PlayerScoreData> BuildLeaderboard(
        bool is_victory, uint8_t winner_id = 0);

    // ========================================================================
    // Member Variables
    // ========================================================================

    Config &config_;
    boost::asio::io_context &io_context_;
    Network network_;
    Engine::registry registry_;
    boost::asio::steady_timer tick_timer_;
    bool running_;

    // Manager components (owned by Server)
    ClientConnectionManager connection_manager_;
    PacketSender packet_sender_;
    PacketHandler packet_handler_;

    // WorldGen components
    std::unique_ptr<worldgen::WorldGenConfigLoader> worldgen_loader_;
    std::unique_ptr<worldgen::WorldGenManager> worldgen_manager_;
    std::unique_ptr<WorldGenSystem> worldgen_system_;

    static constexpr int kTickTimerMs =
        16;  // timer resolution (~60 FPS target)
    int tick_count_;
    std::chrono::steady_clock::time_point last_tick_time_;

    // Player tracking for game over detection
    int total_players_{0};
    int alive_players_{0};

    /**
     * @brief Player death record for tracking victory conditions.
     */
    struct PlayerDeathRecord {
        uint8_t player_id{0};
        std::string username;
        int score{0};
        int death_order{0};  // 0 = still alive, 1 = died first, etc.
        bool is_alive{true};
    };

    /// Map of player_id -> death record for leaderboard
    std::unordered_map<uint8_t, PlayerDeathRecord> player_records_;
    int death_order_counter_{0};  // Increments each time a player dies

    // Game over delay (to let death animation and sound play on client)
    bool game_over_pending_{false};
    float game_over_timer_{0.0f};
    static constexpr float GAME_OVER_DELAY_SEC = 3.0f;  // 3 seconds delay

    // Level selection
    std::string selected_level_uuid_;  ///< Empty = endless mode

    // Victory tracking for finite levels
    bool victory_pending_{false};
    float victory_timer_{0.0f};
    static constexpr float VICTORY_DELAY_SEC = 3.0f;  // 3 seconds delay

    // Singleton instance for system callbacks
    static Server *instance_;
};

}  // namespace server
