#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <audio/IAudioModule.hpp>
#include <config/ConfigLoader.hpp>
#include <loader/DLLoader.hpp>
#include <video/IVideoModule.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/PluginAudioBackend.hpp"
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
        std::cout << "[DEBUG] =================================" << std::endl;
        std::cout << "[DEBUG] Starting R-Type Client with Debug" << std::endl;
        std::cout << "[DEBUG] =================================" << std::endl;
        std::cout << "[Client] Starting R-Type client...\n"
                  << "[Client] Server IP: " << config.server_ip << "\n"
                  << "[Client] TCP Port: " << config.tcp_port << "\n"
                  << "[Client] UDP Port: " << config.udp_port << "\n"
                  << "[Client] Username: " << config.username << std::endl;

        // Load engine configuration
        std::string config_path = "assets/config/engine_config.json";
        if (!Engine::ConfigLoader::Load(config_path)) {
            std::cerr << "[Client] Warning: Failed to load config, using "
                         "defaults"
                      << std::endl;
        }

        // Initialize game world with network parameters
        RC::GameWorld game_world(
            config.server_ip, config.tcp_port, config.udp_port);

        // Load video plugin from config
        std::string video_plugin_path =
            Engine::ConfigLoader::GetVideoBackend();
        std::cout << "[Client] Loading video plugin: " << video_plugin_path
                  << std::endl;

        Engine::DLLoader<Engine::Video::IVideoModule> video_loader;
        video_loader.open(video_plugin_path);
        auto video_module = video_loader.getInstance("entryPoint");

        if (!video_module) {
            throw std::runtime_error("Failed to load video plugin");
        }

        std::cout << "[Client] Loaded video plugin: "
                  << video_module->GetModuleName() << std::endl;

        // Initialize rendering engine with plugin
        auto rendering_engine =
            std::make_unique<Engine::Rendering::RenderingEngine>(video_module);

        if (!rendering_engine->Initialize(
                Engine::ConfigLoader::GetWindowWidth(),
                Engine::ConfigLoader::GetWindowHeight(),
                Engine::ConfigLoader::GetWindowTitle())) {
            throw std::runtime_error("Failed to initialize rendering engine");
        }

        game_world.rendering_engine_ = rendering_engine.get();

        // Load audio plugin from config
        std::string audio_plugin_path =
            Engine::ConfigLoader::GetAudioBackend();
        std::cout << "[Client] Loading audio plugin: " << audio_plugin_path
                  << std::endl;

        Engine::DLLoader<Engine::Audio::IAudioModule> audio_loader;
        audio_loader.open(audio_plugin_path);
        auto audio_module = audio_loader.getInstance("entryPoint");

        if (!audio_module) {
            throw std::runtime_error("Failed to load audio plugin");
        }

        std::cout << "[Client] Loaded audio plugin: "
                  << audio_module->GetModuleName() << std::endl;

        // Initialize audio subsystem with plugin adapter
        auto audio_backend =
            std::make_unique<Audio::PluginAudioBackend>(audio_module);
        Audio::AudioManager audio_manager(std::move(audio_backend));

        // Apply audio settings from config
        audio_manager.SetSfxVolume(Engine::ConfigLoader::GetSfxVolume());
        audio_manager.SetMusicVolume(Engine::ConfigLoader::GetMusicVolume());
        audio_manager.MuteSfx(Engine::ConfigLoader::GetMuteSfx());
        audio_manager.MuteMusic(Engine::ConfigLoader::GetMuteMusic());

        game_world.audio_manager_ = &audio_manager;

        RC::FactoryActors::GetInstance().InitializeEnemyInfoMap("assets/data");

        // Initialize application (registry and scenes)
        RC::ClientApplication::InitializeApplication(game_world);

        // Connect to server with retry mechanism
        if (!RC::ClientApplication::ConnectToServerWithRetry(
                game_world, config)) {
            return EXIT_FAILURE;
        }

        // Clear any spurious events from window creation
        std::cout << "[Client] Clearing spurious events..." << std::endl;
        Engine::Video::Event dummy_event;
        int event_count = 0;
        while (rendering_engine->PollEvent(dummy_event)) {
            event_count++;
            std::cout << "[Client] Discarded event type: "
                      << static_cast<int>(dummy_event.type) << std::endl;
        }
        std::cout << "[Client] Cleared " << event_count << " spurious events"
                  << std::endl;

        // Main game loop using rendering engine
        std::cout << "[Client] Entering main loop..." << std::endl;
        while (rendering_engine->IsWindowOpen()) {
            Engine::Video::Event event;
            while (rendering_engine->PollEvent(event)) {
                if (event.type == Engine::Video::EventType::CLOSED) {
                    std::cout << "[Client] Window close event received"
                              << std::endl;
                    rendering_engine->CloseWindow();
                } else if (event.type ==
                           Engine::Video::EventType::MOUSE_MOVED) {
                    // Update mouse state
                    game_world.mouse_position_.x =
                        static_cast<float>(event.data.mouse_x);
                    game_world.mouse_position_.y =
                        static_cast<float>(event.data.mouse_y);
                } else if (event.type ==
                           Engine::Video::EventType::MOUSE_BUTTON_PRESSED) {
                    if (event.data.mouse_button == 0) {  // Left button
                        game_world.mouse_button_pressed_ = true;
                    }
                } else if (event.type ==
                           Engine::Video::EventType::MOUSE_BUTTON_RELEASED) {
                    if (event.data.mouse_button == 0) {  // Left button
                        game_world.mouse_button_pressed_ = false;
                    }
                }
            }

            // Calculate delta time at the beginning of the frame
            game_world.last_delta_ =
                game_world.delta_time_clock_.Restart().AsSeconds();

            rendering_engine->BeginFrame(Engine::Graphics::Color(
                30, 30, 80, 255));  // Dark blue background

            game_world.registry_.RunSystems();
            rendering_engine->EndFrame();
        }

        rendering_engine->Shutdown();

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
