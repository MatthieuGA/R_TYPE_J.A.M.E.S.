#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void init_registry_systems(Eng::registry &reg, sf::RenderWindow &window) {
    // Set up systems
    reg.add_system<Eng::sparse_array<Com::Controllable>>(controllableSystem);
    reg.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::RigidBody>>(positionSystem);
    reg.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>>(
        [&window](Eng::registry &r,
                Eng::sparse_array<Com::Transform> const &positions,
                Eng::sparse_array<Com::Drawable> &drawables) {
            drawableSystem(r, window, positions, drawables);
        });
}
}  // namespace Rtype::Client
