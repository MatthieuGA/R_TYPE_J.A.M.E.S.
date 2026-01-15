#include <string>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/ColorsConst.hpp"

namespace Rtype::Client {

/**
 * @brief System to update lobby UI elements based on network state.
 */
void LobbyUISystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::LobbyUI> &lobby_uis,
    Eng::sparse_array<Com::Text> &texts,
    Eng::sparse_array<Com::Drawable> &drawables) {
    if (!game_world.server_connection_ ||
        !game_world.server_connection_->is_connected()) {
        return;
    }

    uint8_t connected_count =
        game_world.server_connection_->lobby_connected_count();
    uint8_t ready_count = game_world.server_connection_->lobby_ready_count();
    uint8_t max_players = game_world.server_connection_->lobby_max_players();
    bool is_local_ready =
        game_world.server_connection_->is_local_player_ready();

    for (auto &&[i, lobby_ui] : make_indexed_zipper(lobby_uis)) {
        switch (lobby_ui.ui_type) {
            case Com::LobbyUI::Type::PlayerCount: {
                if (texts[i].has_value()) {
                    std::string new_text =
                        "Players: " + std::to_string(connected_count) + "/" +
                        std::to_string(max_players);
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                    }
                }
                break;
            }
            case Com::LobbyUI::Type::ReadyCount: {
                if (texts[i].has_value()) {
                    std::string new_text =
                        "Ready: " + std::to_string(ready_count) + "/" +
                        std::to_string(connected_count);
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                    }
                }
                break;
            }
            case Com::LobbyUI::Type::ReadyButton: {
                if (texts[i].has_value()) {
                    std::string new_text =
                        is_local_ready ? "Waiting..." : "Ready";
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                    }
                    Engine::Graphics::Color new_color =
                        is_local_ready ? Engine::Graphics::Color(100, 255, 100)
                                       : WHITE_BLUE;
                    if (texts[i]->color != new_color) {
                        texts[i]->color = new_color;
                    }
                }
                if (drawables[i].has_value()) {
                    Engine::Graphics::Color tint =
                        is_local_ready
                            ? Engine::Graphics::Color(100, 200, 100)
                            : Engine::Graphics::Color(255, 255, 255);
                    if (drawables[i]->color != tint) {
                        drawables[i]->color = tint;
                    }
                }
                break;
            }
        }
    }
}

}  // namespace Rtype::Client
