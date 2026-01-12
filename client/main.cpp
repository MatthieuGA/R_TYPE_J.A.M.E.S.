#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/SFMLAudioBackend.hpp"
#include "game/ClientApplication.hpp"
#include "game/CommandLineParser.hpp"
#include "game/ServerSpawner.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"

namespace RC = Rtype::Client;
namespace Audio = Rtype::Client::Audio;

int main(int argc, char *argv[]) {
    try {
        // Parse command-line arguments
        RC::ClientConfig config = RC::CommandLineParser::Parse(argc, argv);

        // Handle solo mode: spawn local server
        if (config.solo_mode) {
            RC::ServerSpawner::SetupSignalHandlers();

            std::cout << "[Client] Starting in solo mode..." << std::endl;

            try {
                uint16_t port = RC::ServerSpawner::SpawnLocalServer();
                config.tcp_port = port;
                config.udp_port = port;
            } catch (const std::exception &e) {
                std::cerr << "[Client] Failed to start local server: "
                          << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        }

        // RAII guard to ensure server cleanup on any exit path
        RC::ServerGuard server_guard(config.solo_mode);

        // Display connection parameters
        std::cout << "[Client] Starting R-Type client...\n"
                  << "[Client] Server IP: " << config.server_ip << "\n"
                  << "[Client] TCP Port: " << config.tcp_port << "\n"
                  << "[Client] UDP Port: " << config.udp_port << "\n"
                  << "[Client] Username: " << config.username << "\n"
                  << "[Client] Mode: "
                  << (config.solo_mode ? "Solo" : "Online") << std::endl;

        // Initialize game world with network parameters
        RC::GameWorld game_world(
            config.server_ip, config.tcp_port, config.udp_port);

        // Initialize audio subsystem with proper lifetime
        // AudioManager must outlive the game loop to prevent dangling pointer
        auto audio_backend = std::make_unique<Audio::SFMLAudioBackend>();
        Audio::AudioManager audio_manager(std::move(audio_backend));
        game_world.audio_manager_ = &audio_manager;

        RC::FactoryActors::GetInstance().InitializeEnemyInfoMap("data/");
        // Initialize application (registry and scenes)
        RC::ClientApplication::InitializeApplication(game_world);

        // Connect to server with retry mechanism
        if (!RC::ClientApplication::ConnectToServerWithRetry(
                game_world, config)) {
            return EXIT_FAILURE;
        }

        // Run the main game loop (includes GAME_START handling)
        RC::ClientApplication::RunGameLoop(game_world);

        // Disconnect gracefully when closing
        if (game_world.server_connection_) {
            std::cout << "[Network] Disconnecting from server..." << std::endl;
            game_world.server_connection_->Disconnect();
        }

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
