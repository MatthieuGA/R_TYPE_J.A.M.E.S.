#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

void InitHealthBar(Com::HealthBar &health_bar) {
    if (!health_bar.foreground_texture.loadFromFile(
            "assets/images/ui/health_bar/foreground_bar.png")) {
        std::cerr << "Failed to load foreground bar texture\n";
        return;
    }
    health_bar.foreground_bar.setTexture(health_bar.foreground_texture);
    if (!health_bar.green_texture.loadFromFile(
            "assets/images/ui/health_bar/green_bar.png")) {
        std::cerr << "Failed to load green bar texture\n";
        return;
    }
    health_bar.green_bar.setTexture(health_bar.green_texture);
    if (!health_bar.yellow_texture.loadFromFile(
            "assets/images/ui/health_bar/yellow_bar.png")) {
        std::cerr << "Failed to load yellow bar texture\n";
        return;
    }
    health_bar.yellow_bar.setTexture(health_bar.yellow_texture);

    health_bar.foreground_bar.setOrigin(
        health_bar.foreground_texture.getSize().x / 2.f,
        health_bar.foreground_texture.getSize().y / 2.f);
    health_bar.green_bar.setOrigin(health_bar.green_texture.getSize().x / 2.f,
        health_bar.green_texture.getSize().y / 2.f);
    health_bar.yellow_bar.setOrigin(
        health_bar.yellow_texture.getSize().x / 2.f,
        health_bar.yellow_texture.getSize().y / 2.f);
    health_bar.is_loaded = true;
}

void DrawHealthBar(Com::HealthBar &health_bar, GameWorld &game_world) {
    game_world.window_.draw(health_bar.yellow_bar);
    game_world.window_.draw(health_bar.green_bar);
    game_world.window_.draw(health_bar.foreground_bar);
}

void UpdatePercentageHealthBar(const Com::Health &health,
    Com::HealthBar &health_bar, GameWorld &game_world) {
    if (health_bar.timer_damage < 1.f) {
        health_bar.timer_damage += game_world.last_delta_;
    } else {
        if (health_bar.percent_delay > health_bar.percent)
            health_bar.percent_delay -= game_world.last_delta_ * 100.f;
        if (health_bar.percent_delay < health_bar.percent)
            health_bar.percent_delay = health_bar.percent;
    }

    health_bar.percent = (static_cast<float>(health.currentHealth) /
                             static_cast<float>(health.maxHealth)) *
                         100.f;
}

void SetHealthsBarSprites(
    const Com::Transform &transform, Com::HealthBar &health_bar) {
    sf::Vector2f transform_s =
        sf::Vector2f(std::abs(transform.scale.x), std::abs(transform.scale.y));

    sf::Vector2f posBar =
        sf::Vector2f(transform.x + (health_bar.offset.x * transform_s.x),
            transform.y + (health_bar.offset.y * transform_s.y));
    health_bar.green_bar.setPosition(posBar);
    health_bar.yellow_bar.setPosition(posBar);
    health_bar.foreground_bar.setPosition(posBar);
    // Update health bar scales based on percent
    health_bar.green_bar.setScale(
        (health_bar.percent / 100.f) * transform_s.x, transform_s.y);
    health_bar.yellow_bar.setScale(
        (health_bar.percent_delay / 100.f) * transform_s.x, transform_s.y);
    health_bar.foreground_bar.setScale(transform_s.x, transform_s.y);
}

void HealthBarSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HealthBar> &health_bars,
    Eng::sparse_array<Com::Health> const &healths) {
    for (auto &&[i, transform, health_bar, health] :
        make_indexed_zipper(transforms, health_bars, healths)) {
        // Initialize health bar sprites if not loaded
        if (!health_bar.is_loaded)
            InitHealthBar(health_bar);
        if (!health_bar.is_loaded)
            return;
        // Update Percentages Bars
        UpdatePercentageHealthBar(health, health_bar, game_world);
        // Update health bar position
        SetHealthsBarSprites(transform, health_bar);
        if (health_bar.percent < 100.f)
            DrawHealthBar(health_bar, game_world);
    }
}
}  // namespace Rtype::Client
