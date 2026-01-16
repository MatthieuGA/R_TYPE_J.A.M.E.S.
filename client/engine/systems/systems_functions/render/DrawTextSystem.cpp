#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "graphics/IRenderContext.hpp"
#include "graphics/SFMLRenderContext.hpp"

namespace Rtype::Client {

const int FONT_SIZE_SCALE = 10;

void InitializeText(Com::Text &text, const Com::Transform &transform) {
    // Backend will load font via render context; mark as initialized
    text.is_loaded = true;
}

void RenderOneTextEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts, GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &text = texts[i];

    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);
    world_position.x += text->offset.x;
    world_position.y += text->offset.y;

    Engine::Graphics::Vector2f world_scale =
        CalculateCumulativeScale(transform.value(), transforms);
    // Scale back by FONT_SIZE_SCALE since character size is already multiplied
    world_scale.x /= FONT_SIZE_SCALE;
    world_scale.y /= FONT_SIZE_SCALE;

    Engine::Graphics::Color engine_color = text->color;
    engine_color.a = static_cast<uint8_t>(text->opacity * 255);

    // Calculate text origin for centering (using GetOffsetFromTransform)
    // Query backend for actual text bounding box
    Engine::Graphics::Vector2f text_bounds(1.0f, 1.0f);
    if (game_world.GetRenderContext()) {
        text_bounds = game_world.GetRenderContext()->GetTextBounds(
            text->font_path.c_str(), text->content.c_str(),
            static_cast<unsigned int>(text->characterSize * FONT_SIZE_SCALE));
    }

    // GetOffsetFromTransform returns negative offsets; negate to get positive
    // for SFML
    Engine::Graphics::Vector2f text_origin =
        GetOffsetFromTransform(transform.value(), text_bounds);
    // Negate to convert from offset to origin (positive coordinates for SFML)
    text_origin.x = -text_origin.x;
    text_origin.y = -text_origin.y;

    Engine::Graphics::DrawableText drawable_text{text->font_path.c_str(),
        text->content.c_str(),
        static_cast<unsigned int>(text->characterSize * FONT_SIZE_SCALE),
        world_position, engine_color, world_scale, text_origin};

    if (game_world.GetRenderContext()) {
        game_world.GetRenderContext()->DrawText(drawable_text);
    } else {
        std::cerr << "RenderContext not set; cannot draw text." << std::endl;
    }
}

void DrawTextRenderSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts) {
    std::vector<int> draw_order;

    for (auto &&[i, transform, text] :
        make_indexed_zipper(transforms, texts)) {
        if (!text.is_loaded)
            InitializeText(text, transform);
        draw_order.push_back(i);
    }

    std::sort(draw_order.begin(), draw_order.end(), [&texts](int a, int b) {
        return texts[a]->z_index < texts[b]->z_index;
    });

    for (auto i : draw_order) {
        RenderOneTextEntity(transforms, texts, game_world, i);
    }
}
}  // namespace Rtype::Client
