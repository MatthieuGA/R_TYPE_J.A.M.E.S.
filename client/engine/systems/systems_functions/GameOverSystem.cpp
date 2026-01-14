/**
 * @file GameOverSystem.cpp
 * @brief System to handle the game over state, display "GAME OVER" text,
 *        and manage fade transition back to lobby.
 */

#include <algorithm>
#include <iostream>
#include <string>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/registry.hpp"
#include "include/sparse_array.hpp"

namespace Rtype::Client {

/**
 * @brief System that manages the game over sequence.
 *
 * When the server sends GAME_END, this system:
 * 1. Shows "GAME OVER" text in big red letters for 2 seconds
 * 2. Fades the screen to black over 1.5 seconds
 * 3. Transitions back to the lobby (main menu)
 *
 * @param reg The ECS registry.
 * @param game_world The game world containing the network connection.
 * @param states Sparse array of GameOverState components.
 * @param texts Sparse array of GameOverText components.
 * @param overlays Sparse array of FadeOverlay components.
 * @param text_comps Sparse array of Text components.
 * @param drawables Sparse array of Drawable components.
 * @param scene_mgmt Sparse array of SceneManagement components.
 */
void GameOverSystem(Engine::registry &reg, GameWorld &game_world,
    Engine::sparse_array<Component::GameOverState> &states,
    Engine::sparse_array<Component::GameOverText> &texts,
    Engine::sparse_array<Component::FadeOverlay> &overlays,
    Engine::sparse_array<Component::Text> &text_comps,
    Engine::sparse_array<Component::Drawable> &drawables,
    Engine::sparse_array<Component::SceneManagement> &scene_mgmt) {
    // Skip if not connected to server
    if (!game_world.server_connection_) {
        return;
    }

    auto &server_connection = *game_world.server_connection_;

    // Find the game over state entity
    for (std::size_t i = 0; i < states.size(); ++i) {
        if (!states.has(i))
            continue;

        auto &state = states[i].value();

        // Check if server sent game end signal
        if (server_connection.HasGameEnded() && !state.is_active) {
            std::cout << "[GameOverSystem] Game ended! Showing GAME OVER text."
                      << std::endl;
            state.is_active = true;
            state.display_timer = 0.0f;
            state.text_phase = true;  // show text phase only
            server_connection.ResetGameEnded();

            if (game_world.audio_manager_) {
                game_world.audio_manager_->StopMusic();
            }

            // Hide the local player's entity (but not other players)
            auto &player_tags = reg.GetComponents<Component::PlayerTag>();
            auto &drawables_all = reg.GetComponents<Component::Drawable>();
            for (std::size_t p = 0; p < player_tags.size(); ++p) {
                if (!player_tags.has(p))
                    continue;
                // Hide drawable if entity has PlayerTag
                if (drawables_all.has(p)) {
                    auto &drawable = drawables_all[p].value();
                    drawable.opacity = 0.0f;  // Make invisible
                }
            }

            // Stop all player movement during game over
            auto &velocities = reg.GetComponents<Component::Velocity>();
            for (std::size_t v = 0; v < velocities.size(); ++v) {
                if (!velocities.has(v))
                    continue;
                auto &vel = velocities[v].value();
                vel.vx = 0.0f;
                vel.vy = 0.0f;
                vel.accelerationX = 0.0f;
                vel.accelerationY = 0.0f;
            }

            // Make the "GAME OVER" text visible and red
            for (std::size_t j = 0; j < texts.size(); ++j) {
                if (!texts.has(j))
                    continue;
                auto &go_text = texts[j].value();
                go_text.visible = true;

                if (text_comps.has(j)) {
                    auto &txt = text_comps[j].value();
                    txt.opacity = 1.0f;  // Fully visible
                    txt.color =
                        Engine::Graphics::Color(255, 0, 0, 255);  // Full red
                }
            }
        }

        if (!state.is_active)
            continue;

        // Use actual delta time from game world
        float delta_time = game_world.last_delta_;

        // Text-only phase: show GAME OVER for configured duration then
        // immediately transition to the lobby (no fade)
        if (state.text_phase) {
            state.display_timer += delta_time;

            if (state.display_timer >=
                Component::GameOverState::kTextDuration) {
                std::cout << "[GameOverSystem] GAME OVER display complete. "
                             "Transitioning to lobby."
                          << std::endl;

                // Hide the GAME OVER text
                for (std::size_t j = 0; j < texts.size(); ++j) {
                    if (!texts.has(j))
                        continue;
                    auto &go_text = texts[j].value();
                    go_text.visible = false;
                    if (text_comps.has(j)) {
                        auto &txt = text_comps[j].value();
                        txt.opacity = 0.0f;
                    }
                }

                // Reset state
                state.is_active = false;
                state.display_timer = 0.0f;
                state.text_phase = true;

                // Trigger scene transition to main menu immediately
                for (std::size_t k = 0; k < scene_mgmt.size(); ++k) {
                    if (!scene_mgmt.has(k))
                        continue;
                    auto &scene = scene_mgmt[k].value();
                    scene.next = "MainMenuScene";
                    std::cout << "[GameOverSystem] Set next scene to "
                                 "MainMenuScene."
                              << std::endl;
                    break;
                }
            }
        }

        float fade_progress = std::min(
            state.fade_timer / Component::GameOverState::kFadeDuration, 1.0f);

        // Update fade overlay opacity (fade to black)
        for (std::size_t j = 0; j < overlays.size(); ++j) {
            if (!overlays.has(j))
                continue;
            auto &overlay = overlays[j].value();
            overlay.alpha = fade_progress * 255.0f;

            // Update drawable opacity and color (black tint)
            if (drawables.has(j)) {
                auto &drawable = drawables[j].value();
                drawable.opacity = fade_progress;
                drawable.color =
                    Engine::Graphics::Color(0, 0, 0, 255);  // Black tint
            }
        }

        // When fade is complete, transition to lobby
        if (state.fade_timer >= Component::GameOverState::kFadeDuration) {
            std::cout << "[GameOverSystem] Fade complete. Transitioning to "
                         "lobby."
                      << std::endl;

            // Reset state
            state.is_active = false;
            state.fade_timer = 0.0f;

            // Reset overlay
            for (std::size_t j = 0; j < overlays.size(); ++j) {
                if (!overlays.has(j))
                    continue;
                auto &overlay = overlays[j].value();
                overlay.alpha = 0.0f;
                if (drawables.has(j)) {
                    auto &drawable = drawables[j].value();
                    drawable.opacity = 0.0f;
                }
            }

            // Trigger scene transition to main menu
            for (std::size_t k = 0; k < scene_mgmt.size(); ++k) {
                if (!scene_mgmt.has(k))
                    continue;
                auto &scene = scene_mgmt[k].value();
                scene.next = "MainMenuScene";
                std::cout << "[GameOverSystem] Set next scene to "
                             "MainMenuScene."
                          << std::endl;
                break;
            }
        }
        break;  // Only process first game over state entity
    }
}

}  // namespace Rtype::Client
