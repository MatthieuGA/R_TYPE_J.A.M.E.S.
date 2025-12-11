#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

const int FONT_SIZE_SCALE = 10;

/**
 * @brief Initialize a text component by loading its font via video backend.
 *
 * @param text Text component to initialize
 * @param game_world Game world containing video backend
 */
void InitializeText(Com::Text &text, GameWorld &game_world) {
    if (!game_world.video_backend_) {
        std::cerr << "ERROR: video_backend is null!" << std::endl;
        return;
    }

    // Load font via video backend
    bool loaded =
        game_world.video_backend_->LoadFont(text.font_id, text.font_path);

    if (!loaded) {
        std::cerr << "ERROR: Failed to load font: " << text.font_path
                  << " (ID: " << text.font_id << ")" << std::endl;
    }

    text.is_loaded = loaded;
}

/**
 * @brief Render a single text entity with transform hierarchy support.
 *
 * @param transforms The sparse array of all transforms
 * @param texts The sparse array of all text components
 * @param game_world The game world containing the render window
 * @param i The entity index to render
 */
void RenderOneTextEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts, GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &text = texts[i];

    if (!game_world.video_backend_) {
        return;
    }

    // Calculate world position with hierarchical rotation
    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);

    // Apply text offset
    world_position.x += text->offset.x;
    world_position.y += text->offset.y;

    // Apply cumulative scale
    float world_scale =
        CalculateCumulativeScale(transform.value(), transforms);

    // Apply color with opacity
    Engine::Graphics::Color final_color = text->color;
    final_color.a = static_cast<uint8_t>(text->opacity * 255);

    // Calculate character size with scale
    unsigned int scaled_char_size =
        static_cast<unsigned int>(text->character_size * world_scale);

    // Get text bounds to calculate origin
    Engine::Graphics::FloatRect text_bounds =
        game_world.video_backend_->GetTextBounds(
            text->content, text->font_id, scaled_char_size);

    // Calculate origin based on transform's origin point
    Engine::Graphics::Vector2f text_size(
        text_bounds.width, text_bounds.height);
    Engine::Graphics::Vector2f origin_offset =
        GetOffsetFromTransform(transform.value(), text_size);

    // Build transform for video backend
    Engine::Video::Transform render_transform;
    render_transform.position = world_position;
    render_transform.rotation = transform->rotationDegrees;
    render_transform.scale =
        Engine::Graphics::Vector2f(1.0f, 1.0f);  // Scale already in char size
    render_transform.origin =
        Engine::Graphics::Vector2f(-origin_offset.x, -origin_offset.y);

    // Draw using video backend
    game_world.video_backend_->DrawText(text->content, text->font_id,
        render_transform, scaled_char_size, final_color);
}

/**
 * @brief System for rendering text objects.
 *
 * Processes all entities with Transform and Text components,
 * loads fonts on first use, sorts by z_index, and renders them.
 *
 * @param reg The entity registry
 * @param game_world The game world context
 * @param transforms The sparse array of transform components
 * @param texts The sparse array of text components
 */
void DrawTextRenderSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts) {
    std::vector<int> draw_order;

    // Collect entities with Transform and Text components
    for (auto &&[i, transform, text] :
        make_indexed_zipper(transforms, texts)) {
        if (!text.is_loaded) {
            InitializeText(text, game_world);
        }
        if (text.is_loaded) {
            draw_order.push_back(i);
        }
    }

    // Sort by z_index
    std::sort(draw_order.begin(), draw_order.end(), [&texts](int a, int b) {
        return texts[a]->z_index < texts[b]->z_index;
    });

    // Render all texts in sorted order
    for (auto i : draw_order) {
        RenderOneTextEntity(transforms, texts, game_world, i);
    }
}
}  // namespace Rtype::Client
