#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <audio/IAudioModule.hpp>
#include <loader/DLLoader.hpp>
#include <video/IVideoModule.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/PluginAudioBackend.hpp"
#include "engine/video/PluginVideoBackend.hpp"
#include "game/InitRegistry.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"

namespace RC = Rtype::Client;
namespace Audio = Rtype::Client::Audio;

int main() {
    try {
        RC::GameWorld game_world;

        // Load video plugin dynamically
        std::string video_plugin_path = "../lib/sfml_video_module.so";
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

        // Initialize video subsystem with plugin adapter
        auto video_backend =
            std::make_unique<Engine::Video::PluginVideoBackend>(video_module);

        if (!video_backend->Initialize(
                RC::WINDOW_WIDTH, RC::WINDOW_HEIGHT, RC::WINDOW_TITLE)) {
            throw std::runtime_error("Failed to initialize video backend");
        }

        game_world.video_backend_ = video_backend.get();

        // Load audio plugin dynamically
        std::string audio_plugin_path = "../lib/sfml_audio_module.so";
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
        game_world.audio_manager_ = &audio_manager;

        std::cout << "[Client] Initializing registry..." << std::endl;
        RC::InitRegistry(game_world, audio_manager);
        std::cout << "[Client] Registry initialized" << std::endl;

        std::cout << "[Client] Initializing scenes..." << std::endl;
        RC::InitSceneLevel(game_world.registry_);
        std::cout << "[Client] Scenes initialized" << std::endl;

        // Clear any spurious events from window creation
        std::cout << "[Client] Clearing spurious events..." << std::endl;
        Engine::Video::Event dummy_event;
        int event_count = 0;
        while (video_backend->PollEvent(dummy_event)) {
            event_count++;
            std::cout << "[Client] Discarded event type: "
                      << static_cast<int>(dummy_event.type) << std::endl;
        }
        std::cout << "[Client] Cleared " << event_count << " spurious events"
                  << std::endl;

        // Main game loop using video backend
        std::cout << "[Client] Entering main loop..." << std::endl;
        while (video_backend->IsWindowOpen()) {
            Engine::Video::Event event;
            while (video_backend->PollEvent(event)) {
                if (event.type == Engine::Video::EventType::CLOSED) {
                    std::cout << "[Client] Window close event received"
                              << std::endl;
                    video_backend->CloseWindow();
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

            video_backend->Clear(Engine::Video::Color(
                30, 30, 80, 255));  // Dark blue background

            game_world.registry_.RunSystems();
            video_backend->Display();
        }

        video_backend->Shutdown();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
