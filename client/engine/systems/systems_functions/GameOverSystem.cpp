/**
 * @file GameOverSystem.cpp
 * @brief System to handle the game over state, display result text,
 *        leaderboard, and manage fade transition back to lobby.
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
 * 1. Shows "GAME OVER" or "VICTORY!" text for 2 seconds
 * 2. Shows leaderboard with player names and scores for 5 seconds
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

    // Get leaderboard texts
    auto &leaderboard_texts = reg.GetComponents<Component::LeaderboardText>();

    // Find the game over state entity
    for (std::size_t i = 0; i < states.size(); ++i) {
        if (!states.has(i))
            continue;

        auto &state = states[i].value();

        // Check if server sent game end signal
        if (server_connection.HasGameEnded() && !state.is_active) {
            // Check if this is a victory for this client
            bool is_victory = server_connection.IsVictory();
            state.is_victory = is_victory;

            if (is_victory) {
                std::cout << "[GameOverSystem] VICTORY! Showing result text."
                          << std::endl;
            } else {
                std::cout << "[GameOverSystem] Game Over! Showing result text."
                          << std::endl;
            }

            state.is_active = true;
            state.display_timer = 0.0f;
            state.leaderboard_timer = 0.0f;
            state.text_phase = true;
            state.leaderboard_phase = false;

            // Don't reset game ended yet - we need the leaderboard data

            if (game_world.audio_manager_) {
                game_world.audio_manager_->StopMusic();
            }

            // Hide all game entities
            auto &player_tags = reg.GetComponents<Component::PlayerTag>();
            auto &drawables_all = reg.GetComponents<Component::Drawable>();
            for (std::size_t p = 0; p < player_tags.size(); ++p) {
                if (!player_tags.has(p))
                    continue;
                if (drawables_all.has(p)) {
                    auto &drawable = drawables_all[p].value();
                    drawable.opacity = 0.0f;
                }
            }

            // Stop all player movement
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

            // Show result text with appropriate message and color
            for (std::size_t j = 0; j < texts.size(); ++j) {
                if (!texts.has(j))
                    continue;
                auto &go_text = texts[j].value();
                go_text.visible = true;

                if (text_comps.has(j)) {
                    auto &txt = text_comps[j].value();
                    txt.opacity = 1.0f;

                    if (is_victory) {
                        txt.content = "VICTORY!";
                        txt.color = Engine::Graphics::Color(0, 255, 0, 255);
                    } else {
                        txt.content = "GAME OVER";
                        txt.color = Engine::Graphics::Color(255, 0, 0, 255);
                    }
                }
            }
        }

        if (!state.is_active)
            continue;

        float delta_time = game_world.last_delta_;

        // Phase 1: Show result text (VICTORY/GAME OVER)
        if (state.text_phase) {
            state.display_timer += delta_time;

            if (state.display_timer >=
                Component::GameOverState::kTextDuration) {
                std::cout << "[GameOverSystem] Result text complete. "
                             "Showing leaderboard."
                          << std::endl;

                // Hide result text
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

                // Transition to leaderboard phase
                state.text_phase = false;
                state.leaderboard_phase = true;
                state.leaderboard_timer = 0.0f;

                // Get and display leaderboard
                auto leaderboard = server_connection.GetLeaderboard();

                // Show leaderboard entries
                for (std::size_t j = 0; j < leaderboard_texts.size(); ++j) {
                    if (!leaderboard_texts.has(j))
                        continue;

                    auto &lb_text = leaderboard_texts[j].value();
                    lb_text.visible = true;

                    if (text_comps.has(j)) {
                        auto &txt = text_comps[j].value();
                        txt.opacity = 1.0f;

                        if (lb_text.rank == 0) {
                            // Title
                            txt.content = "LEADERBOARD";
                            txt.color =
                                Engine::Graphics::Color(255, 255, 0, 255);
                        } else if (lb_text.rank <=
                                   static_cast<int>(leaderboard.size())) {
                            // Player entry
                            size_t idx = static_cast<size_t>(lb_text.rank - 1);
                            const auto &entry = leaderboard[idx];

                            std::string status;
                            if (entry.is_winner) {
                                status = " [WINNER]";
                                txt.color =
                                    Engine::Graphics::Color(0, 255, 0, 255);
                            } else if (entry.death_order == 0) {
                                status = " [ALIVE]";
                                txt.color = Engine::Graphics::Color(
                                    255, 255, 255, 255);
                            } else {
                                status = "";
                                txt.color = Engine::Graphics::Color(
                                    150, 150, 150, 255);
                            }

                            txt.content = std::to_string(lb_text.rank) + ". " +
                                          entry.name + " - " +
                                          std::to_string(entry.score) + status;
                        } else {
                            // No more players
                            txt.content = "";
                            txt.opacity = 0.0f;
                        }
                    }
                }
            }
        }

        // Phase 2: Show leaderboard
        if (state.leaderboard_phase) {
            state.leaderboard_timer += delta_time;

            if (state.leaderboard_timer >=
                Component::GameOverState::kLeaderboardDuration) {
                std::cout << "[GameOverSystem] Leaderboard complete. "
                             "Transitioning to lobby."
                          << std::endl;

                // Hide leaderboard
                for (std::size_t j = 0; j < leaderboard_texts.size(); ++j) {
                    if (!leaderboard_texts.has(j))
                        continue;
                    auto &lb_text = leaderboard_texts[j].value();
                    lb_text.visible = false;
                    if (text_comps.has(j)) {
                        auto &txt = text_comps[j].value();
                        txt.opacity = 0.0f;
                    }
                }

                // Now reset the game ended state
                server_connection.ResetGameEnded();

                // Reset state
                state.is_active = false;
                state.display_timer = 0.0f;
                state.leaderboard_timer = 0.0f;
                state.text_phase = true;
                state.leaderboard_phase = false;

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
        }

        break;  // Only process first game over state entity
    }
}

}  // namespace Rtype::Client
