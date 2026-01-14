#pragma once
#include <memory>
#include <string>
#include <utility>

#include <SFML/Graphics.hpp>
#include <boost/asio.hpp>
#include <graphics/Types.hpp>
#include <input/InputManager.hpp>
#include <time/Clock.hpp>

#include "game/GameAction.hpp"
#include "game/GameInputBindings.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"
#include "input/SFMLInputBackend.hpp"
#include "network/Network.hpp"
#include "platform/IPlatformEventSource.hpp"

#include "engine/events/Event.h"

namespace Rtype::Client::Audio {
class AudioManager;
}

namespace Rtype::Client {

/// Type alias for the game-specific InputManager
using GameInputManager = Engine::Input::InputManager<Game::Action>;

struct GameWorld {
    Engine::registry registry_;
    sf::RenderWindow window_;
    Engine::Graphics::Vector2f window_size_;
    Engine::Time::Clock delta_time_clock_;
    Engine::Time::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    EventBus event_bus_;
    Audio::AudioManager *audio_manager_ = nullptr;

    // Input abstraction layer (templated on Game::Action)
    std::unique_ptr<GameInputManager> input_manager_;

    // Platform event source (backend-agnostic OS event polling)
    std::unique_ptr<Engine::Platform::IPlatformEventSource> event_source_;

    // Network components
    boost::asio::io_context io_context_;
    std::unique_ptr<client::ServerConnection> server_connection_;

    GameWorld(
        const std::string &server_ip, uint16_t tcp_port, uint16_t udp_port)
        : window_(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE) {
        registry_ = Engine::registry();
        delta_time_clock_ = Engine::Time::Clock();
        event_bus_ = EventBus();
        window_size_ =
            Engine::Graphics::Vector2f(static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT));

        // Initialize input manager with SFML backend
        auto sfml_backend = std::make_unique<Input::SFMLInputBackend>(window_);
        input_manager_ =
            std::make_unique<GameInputManager>(std::move(sfml_backend));
        Game::SetupDefaultBindings(*input_manager_);

        // Initialize network connection with provided parameters
        server_connection_ = std::make_unique<client::ServerConnection>(
            io_context_, server_ip, tcp_port, udp_port);
    }
};
}  // namespace Rtype::Client
