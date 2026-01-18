#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

struct PlayerScoreDisplay {
    int id_player;
    int score;
    sf::Text score_text;
};

/**
 * @brief System to manage and render health bars for entities.
 *
 * This system updates health bar percentages based on entity health,
 * positions the health bars according to entity transforms, and
 * renders them to the game window.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world providing delta time and render window
 * @param transforms Sparse array of Transform components
 * @param health_bars Sparse array of HealthBar components
 * @param healths Sparse array of Health components
 */
void ScoreSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::PlayerTag> const &player_tags) {
    static sf::Font font;
    static bool font_loaded = false;
    if (!font_loaded) {
        if (!font.loadFromFile("assets/fonts/dogica.ttf")) {
            std::cerr << "Failed to load font for ScoreSystem\n";
            return;
        }
        font_loaded = true;
    }
    static std::map<int, PlayerScoreDisplay> player_scores;

    for (auto &&[i, player_tag] : make_indexed_zipper(player_tags)) {
        if (player_scores.find(player_tag.id_player) == player_scores.end()) {
            PlayerScoreDisplay display;
            display.id_player = player_tag.id_player;
            display.score = player_tag.score;
            display.score_text.setFont(font);
            display.score_text.setString(std::to_string(player_tag.id_player) +
                                         ": " +
                                         std::to_string(player_tag.score));
            display.score_text.setCharacterSize(24);
            display.score_text.setFillColor(sf::Color::White);
            player_scores.emplace(display.id_player, std::move(display));
        } else {
            auto &entry = player_scores[player_tag.id_player];
            entry.score = player_tag.score;
            entry.score_text.setString(std::to_string(player_tag.id_player) +
                                       ": " +
                                       std::to_string(player_tag.score));
            entry.score_text.setFont(font);
        }
    }

    int index = 0;
    for (auto &[id_player, player_score_display] : player_scores) {
        player_score_display.score_text.setString(
            std::to_string(id_player) + ": " +
            std::to_string(player_score_display.score));
        player_score_display.score_text.setPosition(10.f, 10.f + index * 30.f);
        game_world.GetNativeWindow().draw(player_score_display.score_text);
        index++;
    }
}
}  // namespace Rtype::Client
