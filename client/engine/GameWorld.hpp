/**
 * @file GameWorld.hpp
 * @brief Game world structure containing all game state.
 *
 * NOTE : This file is in transition. The window is now abstracted
 * behind IWindow, but rendering systems still need temporary SFML access.
 * Future PRs will complete the graphics backend abstraction.
 */

#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <boost/asio.hpp>
#include <graphics/IWindow.hpp>
#include <graphics/Types.hpp>
#include <input/InputManager.hpp>
#include <time/Clock.hpp>

#include "game/GameAction.hpp"
#include "game/GameInputBindings.hpp"
#include "graphics/IRenderContext.hpp"
#include "include/AccessibilitySettings.hpp"
#include "include/GameplaySettings.hpp"
#include "include/GraphicsSettings.hpp"
#include "include/WindowConst.hpp"
#include "include/registry.hpp"
#include "input/SFMLInputBackend.hpp"
#include "network/Network.hpp"
#include "platform/IPlatformEventSource.hpp"

#include "engine/events/Event.h"

// Forward declarations (avoid SFML in header)
namespace sf {
class RenderWindow;
}

namespace Rtype::Client::Audio {
class AudioManager;
}

namespace Rtype::Client {

/// Type alias for the game-specific InputManager
using GameInputManager = Engine::Input::InputManager<Game::Action>;

/**
 * @brief Central game state container.
 *
 * Owns the game registry, window, input, networking, and other subsystems.
 *
 * NOTE (PR 1.7 SCOPE): Window is abstracted behind IWindow interface.
 * SFML access is temporarily available via GetNativeWindow() for rendering
 * systems. Future PRs will complete the graphics backend migration.
 */
struct GameWorld {
    Engine::registry registry_;
    std::unique_ptr<Engine::Graphics::IWindow> window_;
    Engine::Graphics::Vector2f window_size_;
    Engine::Time::Clock delta_time_clock_;
    Engine::Time::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    float game_speed_ = 2.0f;  // Game speed multiplier (0.25 to 2.0)
    EventBus event_bus_;
    Audio::AudioManager *audio_manager_ = nullptr;

    // Input rebinding state (for Settings scene)
    std::optional<Game::Action>
        rebinding_action_;  // Current action being rebound (nullopt if not
                            // rebinding)
    bool waiting_for_rebind_key_ =
        false;  // True when waiting for next key press
    std::optional<Engine::entity>
        rebinding_button_entity_;  // Button entity to highlight during rebind

    // Callback for when a key binding is added (for real-time UI refresh)
    std::function<void(Game::Action)> on_binding_added_;

    // Input abstraction layer (templated on Game::Action)
    std::unique_ptr<GameInputManager> input_manager_;

    // Platform event source (backend-agnostic OS event polling)
    std::unique_ptr<Engine::Platform::IPlatformEventSource> event_source_;

    // Network components
    boost::asio::io_context io_context_;
    std::unique_ptr<client::ServerConnection> server_connection_;

    // Callback for when game speed is changed by another player (for UI sync)
    std::function<void(float)> on_external_game_speed_change_;

    // TODO(server-sync): Add callbacks for external difficulty and killable
    // projectiles changes when implementing server synchronization
    // std::function<void(uint8_t)> on_external_difficulty_change_;
    // std::function<void(bool)> on_external_killable_projectiles_change_;

    // Accessibility settings (applies to UI only)
    AccessibilitySettings accessibility_settings_;

    // Graphics settings (resolution, window mode, VSync, frame rate,
    // anti-aliasing)
    GraphicsSettings graphics_settings_;

    // Gameplay settings (auto-fire, difficulty, killable projectiles, etc)
    GameplaySettings gameplay_settings_;

    // Graphics backend (owned by GameWorld as of PR 1.9)
    std::unique_ptr<Engine::Graphics::IRenderContext> render_context_;

    /**
     * @brief Get render context pointer (guaranteed non-null).
     *
     * @return Pointer to the owned render context
     */
    Engine::Graphics::IRenderContext *GetRenderContext() {
        return render_context_.get();
    }

    /**
     * @brief Construct game world with injected window and backend selection.
     *
     * @param window Window interface (ownership transferred)
     * @param backend_name Name of registered graphics backend (e.g., "sfml")
     * @param server_ip Server IP address
     * @param tcp_port TCP port for control messages
     * @param udp_port UDP port for game data
     *
     * @throws std::runtime_error if backend_name is not registered
     */
    GameWorld(std::unique_ptr<Engine::Graphics::IWindow> window,
        const std::string &backend_name, const std::string &server_ip,
        uint16_t tcp_port, uint16_t udp_port);

    /**
     * @brief Get native SFML window reference (TEMPORARY - PR 1.7 SCOPE).
     *
     * This is a temporary accessor for rendering systems that still directly
     * use SFML. Future PRs will migrate these systems to use GraphicsBackend.
     *
     * @return Reference to the underlying sf::RenderWindow
     *
     * TODO(PR 2.0): Remove this once all rendering systems use IRenderContext
     */
    sf::RenderWindow &GetNativeWindow();
};

}  // namespace Rtype::Client
