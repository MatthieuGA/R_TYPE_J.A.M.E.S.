#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Update parallax layer positions according to their scroll speed.
 *
 * Moves parallax layer transforms and wraps them when they go off the left
 * edge to create a looping background. Note: texture size is estimated
 * based on typical parallax layer dimensions.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world providing `last_delta_` and window size
 * @param transforms Sparse array of Transform components
 * @param parallax_layers Sparse array of ParrallaxLayer components
 * @param drawables Sparse array of Drawable components
 */
void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables) {
    // Skip parallax animation when reduced_visuals is enabled
    if (game_world.accessibility_settings_.reduced_visuals) {
        return;
    }

    for (auto &&[i, transform, parallax_layer, drawable] :
        make_indexed_zipper(transforms, parallax_layers, drawables)) {
        if (!drawable.is_loaded)
            continue;
        float dt = game_world.last_delta_;
        transform.x += parallax_layer.scroll_speed * dt;
        // Estimate texture width from common parallax sizes (hardcoded or
        // stored)
        float texture_width = 1920.0f;  // Default parallax width
        if (transform.x <= -(texture_width * transform.scale.x))
            transform.x = game_world.window_size_.x - 2.f;
    }
}
}  // namespace Rtype::Client
