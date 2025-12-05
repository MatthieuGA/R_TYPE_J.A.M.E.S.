#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "registry.hpp"
#include "server/Config.hpp"
#include "server/Network.hpp"

namespace server {

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
    void initialize();

    /**
     * @brief Start the server game loop
     */
    void start();

    /**
     * @brief Update game state (called each tick)
     */
    void update();

    /**
     * @brief Get the ECS registry
     *
     * @return Engine::registry& Reference to the registry
     */
    Engine::registry &getRegistry();

 private:
    /**
     * @brief Register all ECS components
     */
    void registerComponents();

    /**
     * @brief Register all ECS systems
     */
    void registerSystems();

    /**
     * @brief Setup the game tick timer
     */
    void setupGameTick();

    Config &config_;
    boost::asio::io_context &io_context_;
    std::unique_ptr<Network> network_;
    Engine::registry registry_;
    boost::asio::steady_timer tick_timer_;
    bool running_;

    static constexpr int TICK_RATE_MS = 16;  // ~60 FPS
};

}  // namespace server
