#pragma once
#include <cstdint>
#include <vector>

#include <boost/asio.hpp>

#include "include/registry.hpp"
#include "server/ClientConnectionManager.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"
#include "server/PacketHandler.hpp"
#include "server/PacketSender.hpp"

namespace server {

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
     */
    void NotifyPlayerDeath();

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

    static constexpr int TICK_RATE_MS = 16;  // ~60 FPS
    int tick_count_;

    // Player tracking for game over detection
    int total_players_{0};
    int alive_players_{0};

    // Game over delay (to let death animation and sound play on client)
    bool game_over_pending_{false};
    float game_over_timer_{0.0f};
    static constexpr float GAME_OVER_DELAY_SEC = 3.0f;  // 3 seconds delay

    // Singleton instance for system callbacks
    static Server *instance_;
};

}  // namespace server
