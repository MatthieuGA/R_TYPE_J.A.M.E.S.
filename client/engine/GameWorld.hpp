#pragma once
#include <memory>
#include <string>

#include <SFML/Graphics.hpp>
#include <boost/asio.hpp>
#include <graphics/Types.hpp>
#include <time/Clock.hpp>

#include "include/WindowConst.hpp"
#include "include/registry.hpp"
#include "network/Network.hpp"

#include "engine/events/Event.h"

namespace Rtype::Client::Audio {
class AudioManager;
}

namespace Rtype::Client {
struct GameWorld {
    Engine::registry registry_;
    sf::RenderWindow window_;
    Engine::Graphics::Vector2f window_size_;
    Engine::Time::Clock delta_time_clock_;
    Engine::Time::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    EventBus event_bus_;
    Audio::AudioManager *audio_manager_ = nullptr;

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

        // Initialize network connection with provided parameters
        server_connection_ = std::make_unique<client::ServerConnection>(
            io_context_, server_ip, tcp_port, udp_port);
    }
};
}  // namespace Rtype::Client
