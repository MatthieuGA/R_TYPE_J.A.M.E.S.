#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void init_registry_systems(Eng::registry &reg, sf::RenderWindow &window, sf::Clock &deltaTimeClock) {
    // Set up systems
    reg.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>>(
        [&deltaTimeClock](Eng::registry &r,
                Eng::sparse_array<Com::Transform> &positions,
                Eng::sparse_array<Com::Velocity> &velocities) {
            movementSystem(r, deltaTimeClock, positions, velocities);
        });
    reg.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>>(
        [&window](Eng::registry &r,
                Eng::sparse_array<Com::Transform> const &positions,
                Eng::sparse_array<Com::Drawable> &drawables) {
            drawableSystem(r, window, positions, drawables);
        });
    // reset deltaTimeClock if needed in other systems
    reg.add_system<>([&deltaTimeClock](Eng::registry &r) {
        deltaTimeClock.restart();
    });
}
}  // namespace Rtype::Client
