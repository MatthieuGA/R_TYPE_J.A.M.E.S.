#include <string>

#include "adapters/SFMLInputAdapters.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/ColorsConst.hpp"

namespace Rtype::Client {

/**
 * @brief System to update lobby UI elements based on network state.
 *
 * This system updates text and visual elements that display lobby information:
 * - Player count (e.g., "Players: 2/4")
 * - Ready count (e.g., "Ready: 1/2")
 * - Ready button appearance (text and color change when ready)
 *
 * @param reg The entity registry.
 * @param game_world The game world containing the network connection.
 * @param lobby_uis Sparse array of LobbyUI components identifying UI elements.
 * @param texts Sparse array of Text components for updating display text.
 * @param drawables Sparse array of Drawable components for button appearance.
 */
void LobbyUISystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::LobbyUI> &lobby_uis,
    Eng::sparse_array<Com::Text> &texts,
    Eng::sparse_array<Com::Drawable> &drawables) {
    // Skip if not connected to server
    if (!game_world.server_connection_ ||
        !game_world.server_connection_->is_connected()) {
        return;
    }

    // Get current lobby status from network
    uint8_t connected_count =
        game_world.server_connection_->lobby_connected_count();
    uint8_t ready_count = game_world.server_connection_->lobby_ready_count();
    uint8_t max_players = game_world.server_connection_->lobby_max_players();
    bool is_local_ready =
        game_world.server_connection_->is_local_player_ready();

    // Iterate over all lobby UI entities
    for (auto &&[i, lobby_ui] : make_indexed_zipper(lobby_uis)) {
        switch (lobby_ui.ui_type) {
            case Com::LobbyUI::Type::PlayerCount: {
                // Update player count text
                if (texts[i].has_value()) {
                    std::string new_text =
                        "Players: " + std::to_string(connected_count) + "/" +
                        std::to_string(max_players);
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                        texts[i]->text.setString(new_text);
                    }
                }
                break;
            }
            case Com::LobbyUI::Type::ReadyCount: {
                // Update ready count text
                if (texts[i].has_value()) {
                    std::string new_text =
                        "Ready: " + std::to_string(ready_count) + "/" +
                        std::to_string(connected_count);
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                        texts[i]->text.setString(new_text);
                    }
                }
                break;
            }
            case Com::LobbyUI::Type::ReadyButton: {
                // Update ready button text and color
                if (texts[i].has_value()) {
                    std::string new_text =
                        is_local_ready ? "Waiting..." : "Ready";
                    if (texts[i]->content != new_text) {
                        texts[i]->content = new_text;
                        texts[i]->text.setString(new_text);
                    }
                    // Change text color based on ready state
                    Engine::Graphics::Color new_color =
                        is_local_ready ? Engine::Graphics::Color(100, 255, 100)
                                       : WHITE_BLUE;
                    if (texts[i]->color != new_color) {
                        texts[i]->color = new_color;
                        texts[i]->text.setFillColor(
                            Adapters::ToSFMLColor(new_color));
                    }
                }
                // Optionally tint the button sprite
                if (drawables[i].has_value()) {
                    Engine::Graphics::Color tint =
                        is_local_ready
                            ? Engine::Graphics::Color(100, 200, 100)
                            : Engine::Graphics::Color(255, 255, 255);
                    sf::Color sfml_tint = Adapters::ToSFMLColor(tint);
                    if (drawables[i]->sprite.getColor() != sfml_tint) {
                        drawables[i]->sprite.setColor(sfml_tint);
                    }
                }
                break;
            }
        }
    }
}

}  // namespace Rtype::Client
