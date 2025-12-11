#include "game/ClientApplication.hpp"

#include <iostream>

#include <SFML/Graphics.hpp>

#include "game/InitRegistry.hpp"
#include "game/scenes_management/InitScenes.hpp"

namespace Rtype::Client {

bool ClientApplication::ConnectToServerWithRetry(
    GameWorld &game_world, const ClientConfig &config) {
    bool connected = false;

    for (int retry = 0; retry < kMaxRetries && !connected; ++retry) {
        if (retry > 0) {
            std::cout << "[Network] Retry " << retry << "/" << kMaxRetries
                      << "..." << std::endl;
        } else {
            std::cout << "[Network] Attempting to connect to server at "
                      << config.server_ip << ":" << config.tcp_port << "..."
                      << std::endl;
        }

        game_world.server_connection_->ConnectToServer(config.username);

        // Poll io_context to process connection attempt
        for (int i = 0; i < kPollIterations &&
                        !game_world.server_connection_->is_connected();
            ++i) {
            game_world.io_context_.poll();
            sf::sleep(sf::milliseconds(kPollDelayMs));
        }

        connected = game_world.server_connection_->is_connected();

        if (!connected && retry < kMaxRetries - 1) {
            std::cout << "[Network] Connection attempt " << (retry + 1)
                      << " failed. Retrying..." << std::endl;
            // Reset io_context for next attempt
            game_world.io_context_.restart();
            sf::sleep(sf::milliseconds(kRetryDelayMs));
        }
    }

    if (!connected) {
        std::cerr << "[Network] Failed to connect to server after "
                  << kMaxRetries << " attempts." << std::endl;
        std::cerr << "[Network] Please check that the server is running at "
                  << config.server_ip << ":" << config.tcp_port << std::endl;
        return false;
    }

    std::cout << "[Network] Successfully connected to server!" << std::endl;
    return true;
}

void ClientApplication::RunGameLoop(GameWorld &game_world) {
    while (game_world.window_.isOpen()) {
        // Handle window events
        sf::Event event;
        while (game_world.window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                game_world.window_.close();
            }
        }

        // Calculate delta time at the beginning of the frame
        game_world.last_delta_ =
            game_world.delta_time_clock_.restart().asSeconds();

        // Clear, update, and render
        game_world.window_.clear(sf::Color::Black);
        game_world.registry_.RunSystems();
        game_world.window_.display();
    }
}

void ClientApplication::InitializeApplication(GameWorld &game_world) {
    InitRegistry(game_world, *game_world.audio_manager_);
    InitSceneLevel(game_world.registry_);
}

}  // namespace Rtype::Client
