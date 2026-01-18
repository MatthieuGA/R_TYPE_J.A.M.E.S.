/**
 * @file GameWorld.cpp
 * @brief Implementation of GameWorld constructor and methods.
 */

#include "engine/GameWorld.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "graphics/GraphicsBackendFactory.hpp"
#include "platform/SFMLWindow.hpp"

namespace Rtype::Client {

GameWorld::GameWorld(std::unique_ptr<Engine::Graphics::IWindow> window,
    const std::string &backend_name, const std::string &server_ip,
    uint16_t tcp_port, uint16_t udp_port)
    : window_(std::move(window)) {
    registry_ = Engine::registry();
    delta_time_clock_ = Engine::Time::Clock();
    event_bus_ = EventBus();

    auto size = window_->GetSize();
    window_size_ = Engine::Graphics::Vector2f(
        static_cast<float>(size.x), static_cast<float>(size.y));

    // Create graphics backend using factory
    render_context_ = Graphics::GraphicsBackendFactory::Create(
        backend_name, GetNativeWindow());
    if (!render_context_) {
        throw std::runtime_error(
            "Failed to create graphics backend: " + backend_name);
    }

    // Initialize settings manager
    settings_manager_ = std::make_unique<SettingsManager>();

    // Note: input_manager_ and event_source_ will be injected from main.cpp
    // to keep SFML dependencies out of this header
    // LoadSettings() must be called after input_manager_ is set

    // Initialize network connection with provided parameters
    server_connection_ = std::make_unique<client::ServerConnection>(
        io_context_, server_ip, tcp_port, udp_port);

    // Set up callback to sync game_speed_ when another player changes it
    server_connection_->SetOnGameSpeedChanged([this](float speed) {
        game_speed_ = speed;
        // Fire event so UI can update slider position
        if (on_external_game_speed_change_) {
            on_external_game_speed_change_(speed);
        }
    });
}

sf::RenderWindow &GameWorld::GetNativeWindow() {
    // TEMPORARY (PR 1.7 SCOPE): Safe downcast to SFML window
    // This allows rendering systems to keep working during transition
    // TODO(PR 1.8/1.9): Remove once systems use GraphicsBackend
    auto *sfml_window = dynamic_cast<Platform::SFMLWindow *>(window_.get());
    if (!sfml_window) {
        throw std::runtime_error(
            "GameWorld::GetNativeWindow() requires SFMLWindow implementation");
    }
    return sfml_window->GetNativeWindow();
}

void GameWorld::LoadSettings() {
    if (!settings_manager_) {
        std::cerr << "[GameWorld] Settings manager not initialized"
                  << std::endl;
        return;
    }

    if (!input_manager_) {
        std::cerr << "[GameWorld] Cannot load settings: input manager not "
                     "initialized"
                  << std::endl;
        return;
    }

    // Load settings from disk (uses defaults if file missing/invalid)
    settings_manager_->Load(gameplay_settings_, accessibility_settings_,
        graphics_settings_, *input_manager_);
}

void GameWorld::SaveSettings() {
    if (!settings_manager_) {
        std::cerr << "[GameWorld] Settings manager not initialized"
                  << std::endl;
        return;
    }

    if (!input_manager_) {
        std::cerr << "[GameWorld] Cannot save settings: input manager not "
                     "initialized"
                  << std::endl;
        return;
    }

    // Save current settings to disk
    settings_manager_->Save(gameplay_settings_, accessibility_settings_,
        graphics_settings_, *input_manager_);
}

void GameWorld::ApplyGraphicsSettings() {
    if (!window_) {
        std::cerr << "[GameWorld] Cannot apply graphics settings: no window"
                  << std::endl;
        return;
    }

    std::cout << "[GameWorld] Applying graphics settings:" << std::endl;
    std::cout << "  Resolution: " << graphics_settings_.resolution_width << "x"
              << graphics_settings_.resolution_height << std::endl;
    std::cout << "  Window Mode: "
              << (graphics_settings_.window_mode == WindowMode::Fullscreen
                         ? "Fullscreen"
                     : graphics_settings_.window_mode == WindowMode::Borderless
                         ? "Borderless"
                         : "Windowed")
              << std::endl;
    std::cout << "  Anti-Aliasing: "
              << static_cast<int>(graphics_settings_.anti_aliasing)
              << std::endl;
    std::cout << "  VSync: "
              << (graphics_settings_.vsync_enabled ? "ON" : "OFF")
              << std::endl;
    std::cout << "  FPS Limit: " << graphics_settings_.frame_rate_limit
              << std::endl;

    // Cast to SFML window to access Recreate method
    auto *sfml_window = dynamic_cast<Platform::SFMLWindow *>(window_.get());
    if (!sfml_window) {
        std::cerr
            << "[GameWorld] Cannot apply settings: window is not SFMLWindow"
            << std::endl;
        return;
    }

    // Recreate window with new settings
    bool fullscreen =
        (graphics_settings_.window_mode == WindowMode::Fullscreen);
    unsigned int aa_level =
        static_cast<unsigned int>(graphics_settings_.anti_aliasing);

    sfml_window->Recreate(graphics_settings_.resolution_width,
        graphics_settings_.resolution_height, "R-Type Client", fullscreen,
        aa_level);

    // Update window size tracking
    window_size_ = Engine::Graphics::Vector2f(
        static_cast<float>(graphics_settings_.resolution_width),
        static_cast<float>(graphics_settings_.resolution_height));

    // Apply VSync and frame rate limit
    auto &native_window = sfml_window->GetNativeWindow();
    native_window.setVerticalSyncEnabled(graphics_settings_.vsync_enabled);
    native_window.setFramerateLimit(graphics_settings_.frame_rate_limit);

    std::cout << "[GameWorld] Graphics settings applied successfully"
              << std::endl;
}

}  // namespace Rtype::Client
