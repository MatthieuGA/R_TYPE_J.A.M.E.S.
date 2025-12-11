#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Initialize a drawable by loading its texture via video backend.
 *
 * @param drawable Drawable component to initialize
 * @param game_world Game world containing video backend
 */
void InitializeDrawable(Com::Drawable &drawable, GameWorld &game_world) {
    if (!game_world.video_backend_) {
        std::cerr << "[DrawableSystem] ERROR: video_backend is null!"
                  << std::endl;
        return;
    }

    std::cout << "[DrawableSystem] Loading texture: '" << drawable.sprite_path
              << "' with ID: '" << drawable.texture_id << "'" << std::endl;

    // Load texture via video backend
    bool loaded = game_world.video_backend_->LoadTexture(
        drawable.texture_id, drawable.sprite_path);

    if (!loaded) {
        std::cerr << "[DrawableSystem] ERROR: Failed to load texture: "
                  << drawable.sprite_path << " (ID: " << drawable.texture_id
                  << ")" << std::endl;
    } else {
        std::cout << "[DrawableSystem] Successfully loaded texture: "
                  << drawable.texture_id << std::endl;
    }

    drawable.is_loaded = loaded;
}

/**
 * @brief Render one drawable entity using video backend.
 */
void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders, GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];

    if (!game_world.video_backend_) {
        return;
    }

    // Calculate world position with hierarchical rotation
    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);

    float world_scale =
        CalculateCumulativeScale(transform.value(), transforms);

    // Build transform for video backend
    Engine::Video::Transform render_transform;
    render_transform.position = world_position;
    render_transform.rotation = transform->rotationDegrees;
    render_transform.scale =
        Engine::Graphics::Vector2f(world_scale, world_scale);

    // Calculate origin based on sprite size (texture_rect or full texture)
    Engine::Graphics::Vector2f sprite_size;
    if (drawable->texture_rect.width > 0 &&
        drawable->texture_rect.height > 0) {
        // Use texture_rect dimensions (for sprite sheets)
        sprite_size = Engine::Graphics::Vector2f(
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
    } else {
        // Use full texture size
        sprite_size =
            game_world.video_backend_->GetTextureSize(drawable->texture_id);
    }
    Engine::Graphics::Vector2f origin_offset =
        GetOffsetFromTransform(transform.value(), sprite_size);
    render_transform.origin =
        Engine::Graphics::Vector2f(-origin_offset.x, -origin_offset.y);

    // Apply opacity to color
    Engine::Graphics::Color final_color = drawable->color;
    final_color.a = static_cast<uint8_t>(drawable->opacity * 255.0f);

    // Get texture rect if needed (for sprite sheets)
    Engine::Graphics::FloatRect *texture_rect_ptr = nullptr;
    Engine::Graphics::FloatRect texture_rect;
    if (drawable->texture_rect.width > 0 &&
        drawable->texture_rect.height > 0) {
        texture_rect = Engine::Graphics::FloatRect(
            static_cast<float>(drawable->texture_rect.left),
            static_cast<float>(drawable->texture_rect.top),
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
        texture_rect_ptr = &texture_rect;
    }

    // Check if this entity has a shader
    const std::string *shader_id_ptr = nullptr;
    std::string shader_id_str;
    if (shaders.has(i) && shaders[i]->is_loaded) {
        shader_id_str = shaders[i]->shader_id;
        shader_id_ptr = &shader_id_str;

        // Set time uniform for wave effects
        float time = game_world.total_time_clock_.GetElapsedTime().AsSeconds();
        game_world.video_backend_->SetShaderParameter(
            shader_id_str, "time", time);

        // Set other shader uniforms from the shader component
        for (const auto &[uniform_name, uniform_value] :
            shaders[i]->uniforms_float) {
            game_world.video_backend_->SetShaderParameter(
                shader_id_str, uniform_name, uniform_value);
        }
    }

    // Draw using video backend
    game_world.video_backend_->DrawSprite(drawable->texture_id,
        render_transform, texture_rect_ptr, final_color, shader_id_ptr);
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;
    static int frame_count = 0;
    bool debug = (frame_count++ % 60 == 0);

    // Initialize and collect entities with Transform and Drawable components
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.is_loaded) {
            InitializeDrawable(drawable, game_world);
        }
        if (drawable.is_loaded) {
            draw_order.push_back(i);
            if (debug && drawable.texture_id.find("r-typesheet2") !=
                             std::string::npos) {
                std::cout << "[DrawableSystem] Entity " << i
                          << " projectile ready to draw: pos=(" << transform.x
                          << "," << transform.y << "), z=" << drawable.z_index
                          << ", rect=(" << drawable.texture_rect.width << "x"
                          << drawable.texture_rect.height << ")" << std::endl;
            }
        }
    }

    // Sort by z_index
    std::sort(
        draw_order.begin(), draw_order.end(), [&drawables](int a, int b) {
            return drawables[a]->z_index < drawables[b]->z_index;
        });

    // Render all entities in order
    for (auto i : draw_order) {
        RenderOneEntity(transforms, drawables, shaders, game_world, i);
    }
}

}  // namespace Rtype::Client
