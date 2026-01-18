#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/SFMLAudioBackend.hpp"
#include "game/ClientApplication.hpp"
#include "game/CommandLineParser.hpp"
#include "game/ServerSpawner.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "graphics/BackendResolver.hpp"
#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/GraphicsPluginLoader.hpp"
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

        // Initialize game world with window and network parameters
        auto window = std::make_unique<RC::Platform::SFMLWindow>(
            RC::WINDOW_WIDTH, RC::WINDOW_HEIGHT, RC::WINDOW_TITLE);

        // Initialize platform event source (SFML backend)
        auto *sfml_window =
            dynamic_cast<RC::Platform::SFMLWindow *>(window.get());
        if (!sfml_window) {
            throw std::runtime_error("SFMLWindow implementation required");
        }

        // Create input backend
        auto sfml_input_backend =
            std::make_unique<RC::Input::SFMLInputBackend>(
                sfml_window->GetNativeWindow());

        // We'll create the event source after moving the window into
        // GameWorld; keep a pointer to the native SFML window and create
        // SFMLEventSource from it later.

        // Determine which graphics backend to use
        // Default to "sfml" if not specified via CLI
        std::string backend_name =
            config.graphics_backend.empty() ? "sfml" : config.graphics_backend;

        // Register static SFML backend (always available as fallback)
        RC::Graphics::GraphicsBackendFactory::Register(
            "sfml", [](sf::RenderWindow &window) {
                return std::make_unique<RC::Graphics::SFMLRenderContext>(
                    window);
            });

        // Resolve the requested backend (may load plugin or use static)
        if (!RC::Graphics::ResolveGraphicsBackend(
                backend_name, "./plugins", "sfml")) {
            return EXIT_FAILURE;
        }

        // If resolution succeeded, use the backend
        // (it's now in the factory registry)

        // Create game world with resolved graphics backend
        RC::GameWorld game_world(std::move(window), backend_name,
            config.server_ip, config.tcp_port, config.udp_port);

        // Inject dependencies
        game_world.input_manager_ = std::make_unique<RC::GameInputManager>(
            std::move(sfml_input_backend));
        Game::SetupDefaultBindings(*game_world.input_manager_);

        // Initialize platform event source (SFML backend)
        game_world.event_source_ =
            std::make_unique<RC::Platform::SFMLEventSource>(
                game_world.GetNativeWindow());

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
