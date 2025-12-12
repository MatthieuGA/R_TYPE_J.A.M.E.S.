#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Initialize the health bar textures via RenderingEngine.
 *
 * @param health_bar The HealthBar component to initialize.
 * @param game_world Game world containing rendering engine.
 */
void InitHealthBar(Com::HealthBar &health_bar, GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        std::cerr << "[HealthBarSystem] ERROR: rendering_engine is null!" << std::endl;
        return;
    }

    // Load health bar textures
    bool green_loaded = game_world.rendering_engine_->LoadTexture(
        health_bar.green_texture_id,
        "assets/images/" + health_bar.green_texture_id);
    bool yellow_loaded = game_world.rendering_engine_->LoadTexture(
        health_bar.yellow_texture_id,
        "assets/images/" + health_bar.yellow_texture_id);
    bool foreground_loaded = game_world.rendering_engine_->LoadTexture(
        health_bar.foreground_texture_id,
        "assets/images/" + health_bar.foreground_texture_id);

    if (!green_loaded || !yellow_loaded || !foreground_loaded) {
        std::cerr << "[HealthBarSystem] ERROR: Failed to load health bar textures" << std::endl;
        return;
    }

    health_bar.is_loaded = true;
}

/**
 * @brief Draw the health bar components using RenderingEngine.
 *
 * Renders bars with center origin in pixel coordinates (like SFML setOrigin).
 * This keeps bars centered as they scale in width - scaling from center
 * means they expand/shrink equally in both directions.
 *
 * @param health_bar The HealthBar component to draw.
 * @param position World position for the health bar center.
 * @param scale Base scale for the health bar.
 * @param game_world The GameWorld providing the rendering engine.
 */
void DrawHealthBar(Com::HealthBar &health_bar,
    const Engine::Graphics::Vector2f &position,
    const Engine::Graphics::Vector2f &base_scale,
    GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        return;
    }

    // Get texture sizes to calculate center origin in pixels
    Engine::Graphics::Vector2f yellow_size =
        game_world.rendering_engine_->GetTextureSize(health_bar.yellow_texture_id);
    Engine::Graphics::Vector2f green_size =
        game_world.rendering_engine_->GetTextureSize(health_bar.green_texture_id);
    Engine::Graphics::Vector2f foreground_size =
        game_world.rendering_engine_->GetTextureSize(health_bar.foreground_texture_id);

    // Calculate center origin in pixels (like SFML setOrigin(width/2, height/2))
    // NOTE: RenderingEngine negates origin_offset, so we pass negative to get positive in SFML
    Engine::Graphics::Vector2f yellow_origin(-yellow_size.x / 2.f, -yellow_size.y / 2.f);
    Engine::Graphics::Vector2f green_origin(-green_size.x / 2.f, -green_size.y / 2.f);
    Engine::Graphics::Vector2f foreground_origin(-foreground_size.x / 2.f, -foreground_size.y / 2.f);

    // Calculate scaled percentages for width
    Engine::Graphics::Vector2f yellow_scale(
        (health_bar.percent_delay / 100.f) * base_scale.x, base_scale.y);
    Engine::Graphics::Vector2f green_scale(
        (health_bar.percent / 100.f) * base_scale.x, base_scale.y);

    // Render bars back to front (yellow -> green -> foreground)
    // Using pixel-coordinate origins so bars scale from center
    
    // Yellow bar (delayed damage indicator)
    game_world.rendering_engine_->RenderSprite(
        health_bar.yellow_texture_id, position, yellow_scale, 0.0f,
        nullptr, Engine::Graphics::Color::White, yellow_origin, nullptr);

    // Green bar (current health)
    game_world.rendering_engine_->RenderSprite(
        health_bar.green_texture_id, position, green_scale, 0.0f,
        nullptr, Engine::Graphics::Color::White, green_origin, nullptr);

    // Foreground border (full width)
    game_world.rendering_engine_->RenderSprite(
        health_bar.foreground_texture_id, position, base_scale, 0.0f,
        nullptr, Engine::Graphics::Color::White, foreground_origin, nullptr);
}

/**
 * @brief Update the health bar percentages based on current health.
 *
 * @param health The Health component of the entity.
 * @param health_bar The HealthBar component to update.
 * @param game_world The GameWorld providing delta time.
 */
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

/**
 * @brief Calculate the world position for the health bar.
 *
 * @param transform The Transform component of the entity.
 * @param health_bar The HealthBar component.
 * @return World position for the health bar.
 */
Engine::Graphics::Vector2f CalculateHealthBarPosition(
    const Com::Transform &transform, const Com::HealthBar &health_bar) {
    Engine::Graphics::Vector2f transform_scale(
        std::abs(transform.scale.x), std::abs(transform.scale.y));

    return Engine::Graphics::Vector2f(
        transform.x + (health_bar.offset.x * transform_scale.x),
        transform.y + (health_bar.offset.y * transform_scale.y));
}

/**
 * @brief System to manage and render health bars for entities.
 *
 * This system updates health bar percentages based on entity health,
 * positions the health bars according to entity transforms, and
 * renders them using the RenderingEngine API.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world providing delta time and rendering engine
 * @param transforms Sparse array of Transform components
 * @param health_bars Sparse array of HealthBar components
 * @param healths Sparse array of Health components
 */
void HealthBarSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HealthBar> &health_bars,
    Eng::sparse_array<Com::Health> const &healths) {
    for (auto &&[i, transform, health_bar, health] :
        make_indexed_zipper(transforms, health_bars, healths)) {
        // Initialize health bar textures if not loaded
        if (!health_bar.is_loaded) {
            InitHealthBar(health_bar, game_world);
        }
        if (!health_bar.is_loaded) {
            continue;
        }

        // Update health bar percentages
        UpdatePercentageHealthBar(health, health_bar, game_world);

        // Calculate position and scale
        Engine::Graphics::Vector2f bar_position =
            CalculateHealthBarPosition(transform, health_bar);
        Engine::Graphics::Vector2f bar_scale(
            std::abs(transform.scale.x), std::abs(transform.scale.y));

        // Only draw if health is not full
        if (health_bar.percent < 100.f) {
            DrawHealthBar(health_bar, bar_position, bar_scale, game_world);
        }
    }
}
}  // namespace Rtype::Client
