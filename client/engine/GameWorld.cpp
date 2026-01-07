/**
 * @file GameWorld.cpp
 * @brief Implementation of GameWorld constructor and methods.
 */

#include "engine/GameWorld.hpp"

#include <memory>
#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "platform/SFMLWindow.hpp"

namespace Rtype::Client {

GameWorld::GameWorld(std::unique_ptr<Engine::Graphics::IWindow> window,
    const std::string &server_ip, uint16_t tcp_port, uint16_t udp_port)
    : window_(std::move(window)) {
    registry_ = Engine::registry();
    delta_time_clock_ = Engine::Time::Clock();
    event_bus_ = EventBus();
    window_size_ = Engine::Graphics::Vector2f(
        static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));

    // Note: input_manager_ and event_source_ will be injected from main.cpp
    // to keep SFML dependencies out of this header

    // Initialize network connection with provided parameters
    server_connection_ = std::make_unique<client::ServerConnection>(
        io_context_, server_ip, tcp_port, udp_port);
}

sf::RenderWindow &GameWorld::GetNativeWindow() {
    // TEMPORARY (PR 1.7 SCOPE): Direct cast to SFML window
    // This allows rendering systems to keep working during transition
    // TODO(PR 1.8/1.9): Remove once systems use GraphicsBackend
    auto *sfml_window = static_cast<Platform::SFMLWindow *>(window_.get());
    return sfml_window->GetNativeWindow();
}

}  // namespace Rtype::Client
