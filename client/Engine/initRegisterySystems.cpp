#include "Engine/initRegisterySystems.hpp"
#include "include/indexed_zipper.hpp"

using namespace Engine;

namespace Rtype::Client {
    using namespace Component;
void init_registry_systems(registry &reg, sf::RenderWindow &window) {
    // Set up systems
    reg.add_system<sparse_array<Controllable>>(controllableSystem);
    reg.add_system<sparse_array<Transform>,
                    sparse_array<RigidBody>>(positionSystem);
    reg.add_system<sparse_array<Transform>, sparse_array<Drawable>>(
        [&window](registry &r,
                sparse_array<Transform> const &positions,
                sparse_array<Drawable> &drawables) {
            drawableSystem(r, window, positions, drawables);
        });
}
}  // namespace Rtype::Client
