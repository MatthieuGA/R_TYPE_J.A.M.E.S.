#pragma once
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "graphics/Types.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"
#include "network/Network.hpp"
#include "rendering/RenderingEngine.hpp"
#include "time/Clock.hpp"

#include "engine/events/Event.h"

namespace Rtype::Client::Audio {
class AudioManager;
}

namespace Rtype::Client {
/**
 * @brief Central game state container.
 *
 * Holds the ECS registry, rendering engine, audio backend, timing, network,
 * and event bus. Completely backend-agnostic - no SFML types.
 */
struct GameWorld {
    Engine::registry registry_;
    Engine::Rendering::RenderingEngine *rendering_engine_ = nullptr;
    Engine::Graphics::Vector2f window_size_;
    Engine::Time::Clock delta_time_clock_;
    Engine::Time::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    EventBus event_bus_;
    Audio::AudioManager *audio_manager_ = nullptr;

    // Input state
    Engine::Graphics::Vector2f mouse_position_{0.0f, 0.0f};
    bool mouse_button_pressed_ = false;

    // Network components
    boost::asio::io_context io_context_;
    std::unique_ptr<client::ServerConnection> server_connection_;

    GameWorld(
        const std::string &server_ip, uint16_t tcp_port, uint16_t udp_port) {
        registry_ = Engine::registry();
        event_bus_ = EventBus();
        window_size_ =
            Engine::Graphics::Vector2f(static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT));

        // Initialize network connection with provided parameters
        server_connection_ = std::make_unique<client::ServerConnection>(
            io_context_, server_ip, tcp_port, udp_port);

        // Note: rendering_engine_ must be set externally after construction
    }
};
}  // namespace Rtype::Client
