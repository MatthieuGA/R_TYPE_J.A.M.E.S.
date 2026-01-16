/**
 * @file GameWorld.cpp
 * @brief Implementation of GameWorld constructor and methods.
 */

#include "engine/GameWorld.hpp"

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

    // Note: input_manager_ and event_source_ will be injected from main.cpp
    // to keep SFML dependencies out of this header

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

    // Set up callback to sync difficulty when another player changes it
    server_connection_->SetOnDifficultyChanged([this](uint8_t difficulty) {
        gameplay_settings_.difficulty =
            static_cast<DifficultyLevel>(difficulty);
        // Fire event so UI can update button highlighting
        if (on_external_difficulty_change_) {
            on_external_difficulty_change_(difficulty);
        }
    });

    // Set up callback to sync killable projectiles when another player changes
    // it
    server_connection_->SetOnKillableProjectilesChanged([this](bool enabled) {
        gameplay_settings_.killable_enemy_projectiles = enabled;
        // Fire event so UI can update button text
        if (on_external_killable_projectiles_change_) {
            on_external_killable_projectiles_change_(enabled);
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

}  // namespace Rtype::Client
