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
};

}  // namespace server
