#pragma once
#include <graphics/Types.hpp>
#include <time/Clock.hpp>

#include "engine/video/PluginVideoBackend.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"

#include "engine/events/Event.h"

namespace Rtype::Client::Audio {
class AudioManager;
}

namespace Rtype::Client {
/**
 * @brief Central game state container.
 *
 * Holds the ECS registry, video/audio backends, timing, and event bus.
 * Now completely backend-agnostic - no SFML types.
 */
struct GameWorld {
    Engine::registry registry_;
    Engine::Video::PluginVideoBackend *video_backend_ = nullptr;
    Engine::Graphics::Vector2f window_size_;
    Engine::Time::Clock delta_time_clock_;
    Engine::Time::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    EventBus event_bus_;
    Audio::AudioManager *audio_manager_ = nullptr;

    // Input state
    Engine::Graphics::Vector2f mouse_position_{0.0f, 0.0f};
    bool mouse_button_pressed_ = false;

    GameWorld() {
        registry_ = Engine::registry();
        event_bus_ = EventBus();
        window_size_ =
            Engine::Graphics::Vector2f(static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT));
        // Note: video_backend_ must be set externally after construction
    }
};

}  // namespace Rtype::Client
