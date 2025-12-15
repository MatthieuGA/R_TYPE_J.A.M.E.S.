#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Update parallax layer positions according to their scroll speed.
 *
 * Moves parallax layer transforms and wraps them when they go off the left
 * edge to create a looping background.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world providing `last_delta_`, window size, and video
 * backend
 * @param transforms Sparse array of Transform components
 * @param parallax_layers Sparse array of ParrallaxLayer components
 * @param drawables Sparse array of Drawable components (for texture ID)
 */
void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables) {
    (void)reg;

    if (!game_world.rendering_engine_) {
        return;
    }

    for (auto &&[i, transform, parallax_layer, drawable] :
        make_indexed_zipper(transforms, parallax_layers, drawables)) {
        if (!drawable.is_loaded)
            continue;

        float dt = game_world.last_delta_;

        // Update position based on parallax scroll speed
        transform.x += parallax_layer.scroll_speed * dt;

        // Get texture size from video backend for wrapping
        Engine::Graphics::Vector2f texture_size =
            game_world.rendering_engine_->GetTextureSize(drawable.texture_id);

        // Wrap around when the texture scrolls off screen
        if (transform.x <= -(texture_size.x * transform.scale.x)) {
            transform.x = game_world.window_size_.x - 2.f;
        }
    }
}
}  // namespace Rtype::Client
