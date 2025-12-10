#include <iostream>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "game/CommandLineParser.hpp"
#include "game/InitRegistry.hpp"
#include "game/scenes_management/InitScenes.hpp"

namespace RC = Rtype::Client;

int main(int argc, char *argv[]) {
    try {
        // Parse command-line arguments
        RC::ClientConfig config = RC::CommandLineParser::Parse(argc, argv);

        // Display connection parameters
        std::cout << "[Client] Starting R-Type client...\n"
                  << "[Client] Server IP: " << config.server_ip << "\n"
                  << "[Client] TCP Port: " << config.tcp_port << "\n"
                  << "[Client] UDP Port: " << config.udp_port << "\n"
                  << "[Client] Username: " << config.username << std::endl;

        // Initialize game world with network parameters
        RC::GameWorld game_world(
            config.server_ip, config.tcp_port, config.udp_port);

        RC::InitRegistry(game_world);
        RC::InitSceneLevel(game_world.registry_);

        // Connect to server
        std::cout << "[Network] Attempting to connect to server at "
                  << config.server_ip << ":" << config.tcp_port << "..."
                  << std::endl;
        game_world.server_connection_->ConnectToServer(config.username);

        // Poll io_context to process connection attempt
        // Give it a few iterations to complete the handshake
        for (int i = 0;
            i < 10 && !game_world.server_connection_->is_connected(); ++i) {
            game_world.io_context_.poll();
            sf::sleep(sf::milliseconds(50));  // Small delay between polls
        }

        if (!game_world.server_connection_->is_connected()) {
            std::cerr << "[Network] Failed to connect to server (timeout)"
                      << std::endl;
        }

        while (game_world.window_.isOpen()) {
            sf::Event event;
            while (game_world.window_.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    game_world.window_.close();
            }
            // Calculate delta time at the beginning of the frame
            game_world.last_delta_ =
                game_world.delta_time_clock_.restart().asSeconds();

            game_world.window_.clear(sf::Color::Black);
            game_world.registry_.RunSystems();
            game_world.window_.display();
        }

        // Disconnect gracefully when closing
        if (game_world.server_connection_) {
            std::cout << "[Network] Disconnecting from server..." << std::endl;
            game_world.server_connection_->Disconnect();
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
