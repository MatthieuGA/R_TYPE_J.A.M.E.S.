#include <iostream>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

/**
 * @brief Clamp entity transforms to remain inside the render window.
 *
 * This system enforces that entities (typically players) do not leave the
 * visible playfield by clamping their `x` and `y` coordinates to the window
 * bounds.
 *
 * @param reg Engine registry (unused)
 * @param window SFML render window used to obtain size limits
 * @param transforms Sparse array of Transform components to clamp
 * @param player_tags Sparse array of PlayerTag components (filter)
 */
void PlayfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags) {
    for (auto &&[i, transform, player_tag] :
        make_indexed_zipper(transforms, player_tags)) {
        // Update position based on velocity
        if (transform.x < 0)
            transform.x = 0;
        if (transform.y < 0)
            transform.y = 0;
        if (transform.x > window.getSize().x)
            transform.x = static_cast<float>(window.getSize().x);
        if (transform.y > window.getSize().y)
            transform.y = static_cast<float>(window.getSize().y);
    }
}
}  // namespace Rtype::Client
