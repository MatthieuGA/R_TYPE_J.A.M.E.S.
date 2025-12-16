#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/SFMLAudioBackend.hpp"
#include "game/ClientApplication.hpp"
#include "game/CommandLineParser.hpp"
#include "game/InitRegistry.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/registry.hpp"

namespace RC = Rtype::Client;
namespace Audio = Rtype::Client::Audio;

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
