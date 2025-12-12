#include "game/ClientApplication.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include <graphics/Types.hpp>
#include <video/IVideoModule.hpp>

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
            // TODO(copilot): Replace SFML sleep with backend-agnostic sleep
            // sf::sleep(sf::milliseconds(kPollDelayMs));
            std::this_thread::sleep_for(
                std::chrono::milliseconds(kPollDelayMs));
        }

        connected = game_world.server_connection_->is_connected();

        if (!connected && retry < kMaxRetries - 1) {
            std::cout << "[Network] Connection attempt " << (retry + 1)
                      << " failed. Retrying..." << std::endl;
            // Reset io_context for next attempt
            game_world.io_context_.restart();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(kRetryDelayMs));
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
    if (!game_world.rendering_engine_) {
        std::cerr << "[ClientApplication] ERROR: rendering_engine is null!"
                  << std::endl;
        return;
    }

    while (game_world.rendering_engine_->IsWindowOpen()) {
        // Handle window events
        Engine::Video::Event event;
        while (game_world.rendering_engine_->PollEvent(event)) {
            if (event.type == Engine::Video::EventType::CLOSED) {
                game_world.rendering_engine_->CloseWindow();
            }
        }

        // Calculate delta time at the beginning of the frame
        game_world.last_delta_ =
            game_world.delta_time_clock_.Restart().AsSeconds();

        // Begin frame, update systems, end frame
        game_world.rendering_engine_->BeginFrame(
            Engine::Graphics::Color(0, 0, 0, 255));
        game_world.registry_.RunSystems();
        game_world.rendering_engine_->EndFrame();
    }
}

void ClientApplication::InitializeApplication(GameWorld &game_world) {
    InitRegistry(game_world, *game_world.audio_manager_);
    InitSceneLevel(game_world.registry_);
}

}  // namespace Rtype::Client
