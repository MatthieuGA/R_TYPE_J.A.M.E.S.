#pragma once

#include "engine/GameWorld.hpp"
#include "game/CommandLineParser.hpp"

namespace Rtype::Client {

/**
 * @brief Application-level functions for R-Type client initialization and
 * execution.
 */
class ClientApplication {
 public:
    /**
     * @brief Connect to the server with automatic retry mechanism.
     *
     * Attempts to establish a connection to the server with multiple retries.
     * Provides user feedback during the connection process.
     *
     * @param game_world The game world containing the server connection
     * @param config Client configuration with server details
     * @return true if connection succeeded, false otherwise
     */
    static bool ConnectToServerWithRetry(
        GameWorld &game_world, const ClientConfig &config);

    /**
     * @brief Run the main game loop.
     *
     * Handles event processing, game logic updates, and rendering until the
     * window is closed.
     *
     * @param game_world The game world to run
     */
    static void RunGameLoop(GameWorld &game_world);

    /**
     * @brief Initialize the client application.
     *
     * Sets up the game world, registry, and initial scene.
     *
     * @param game_world The game world to initialize
     */
    static void InitializeApplication(GameWorld &game_world);

 private:
    // Connection retry constants
    static constexpr int kMaxRetries = 3;
    static constexpr int kPollIterations = 20;
    static constexpr int kPollDelayMs = 100;
    static constexpr int kRetryDelayMs = 500;
};

}  // namespace Rtype::Client
