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
#include "graphics/SFMLRenderContext.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"
#include "input/SFMLInputBackend.hpp"
#include "platform/SFMLEventSource.hpp"
#include "platform/SFMLWindow.hpp"

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

        // Initialize game world with window and network parameters
        auto window = std::make_unique<RC::Platform::SFMLWindow>(
            RC::WINDOW_WIDTH, RC::WINDOW_HEIGHT, RC::WINDOW_TITLE);

        // Initialize platform event source (SFML backend)
        auto *sfml_window =
            dynamic_cast<RC::Platform::SFMLWindow *>(window.get());
        if (!sfml_window) {
            throw std::runtime_error("SFMLWindow implementation required");
        }
        auto event_source = std::make_unique<RC::Platform::SFMLEventSource>(
            sfml_window->GetNativeWindow());

        // Create input backend (reuse sfml_window pointer)
        auto sfml_input_backend =
            std::make_unique<RC::Input::SFMLInputBackend>(
                sfml_window->GetNativeWindow());

        // Graphics backend: SFMLRenderContext (lives for game loop scope)
        auto render_context =
            std::make_unique<RC::Graphics::SFMLRenderContext>(
                sfml_window->GetNativeWindow());

        RC::GameWorld game_world(std::move(window), config.server_ip,
            config.tcp_port, config.udp_port);

        // Wire render context into game world
        game_world.SetRenderContext(render_context.get());

        // Inject dependencies
        game_world.event_source_ = std::move(event_source);
        game_world.input_manager_ = std::make_unique<RC::GameInputManager>(
            std::move(sfml_input_backend));
        Game::SetupDefaultBindings(*game_world.input_manager_);

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
