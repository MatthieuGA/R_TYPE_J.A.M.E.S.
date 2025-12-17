#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

// Conversion helpers between Engine and SFML types at rendering boundary
// Should be removed after the migration to Engine Graphics is complete
inline sf::Vector2f ToSFML(const Engine::Graphics::Vector2f &v) {
    return sf::Vector2f(v.x, v.y);
}

inline Engine::Graphics::Vector2f ToEngine(const sf::Vector2f &v) {
    return Engine::Graphics::Vector2f(v.x, v.y);
}

const int FONT_SIZE_SCALE = 10;

/**
 * @brief Initialize a text component with font loading.
 *
 * Loads the font and sets up the sf::Text object with the content,
 * character size, and color from the Text component.
 *
 * @param text Text component to initialize
 * @param transform Transform for origin calculation
 */
void InitializeText(Com::Text &text, const Com::Transform &transform) {
    if (!text.font.loadFromFile(text.fontPath)) {
        std::cerr << "ERROR: Failed to load font from " << text.fontPath
                  << "\n";
    } else {
        text.text.setFont(text.font);
        text.text.setString(text.content);
        text.text.setCharacterSize(text.characterSize * FONT_SIZE_SCALE);
        text.text.setFillColor(text.color);

        // Set origin based on transform's origin point
        sf::FloatRect bounds = text.text.getLocalBounds();
        Engine::Graphics::Vector2f origin = GetOffsetFromTransform(transform,
            Engine::Graphics::Vector2f(bounds.width, bounds.height));
        text.text.setOrigin(-ToSFML(origin));
    }
    text.is_loaded = true;
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

    // Calculate world position with hierarchical rotation
    sf::Vector2f world_position = ToSFML(
        CalculateWorldPositionWithHierarchy(transform.value(), transforms));
    world_position.x += text->offset.x;
    world_position.y += text->offset.y;
    text->text.setPosition(world_position);

    // Apply cumulative scale
    sf::Vector2f world_scale =
        ToSFML(CalculateCumulativeScale(transform.value(), transforms));
    world_scale.x /= FONT_SIZE_SCALE;
    world_scale.y /= FONT_SIZE_SCALE;
    text->text.setScale(world_scale);

    // Apply rotation
    text->text.setRotation(transform->rotationDegrees);

    // Apply color with opacity
    sf::Color color = text->color;
    color.a = static_cast<sf::Uint8>(text->opacity * 255);
    text->text.setFillColor(color);

    // Draw text
    game_world.window_.draw(text->text);
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
        if (!text.is_loaded)
            InitializeText(text, transform);
        draw_order.push_back(i);
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
