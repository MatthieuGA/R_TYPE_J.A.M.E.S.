#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Update parallax layer positions according to their scroll speed.
 *
 * Moves parallax layer transforms and wraps them when they go off the left
 * edge to create a looping background.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world providing `last_delta_` and window size
 * @param transforms Sparse array of Transform components
 * @param parallax_layers Sparse array of ParrallaxLayer components
 * @param drawables Sparse array of Drawable components (for texture size)
 */
void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables) {
    for (auto &&[i, tranform, parallax_layers, drawable] :
        make_indexed_zipper(transforms, parallax_layers, drawables)) {
        if (!drawable.isLoaded) continue;
        float dt = game_world.last_delta_;
        // Update position based on parallax scroll speed
        tranform.x += parallax_layers.scroll_speed * dt;
        if (tranform.x <= -(drawable.texture.getSize().x * tranform.scale)) {
            tranform.x = game_world.window_size_.x - 2.f;
        }
    }
}
}  // namespace Rtype::Client
