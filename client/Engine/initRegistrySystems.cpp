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
                Eng::sparse_array<Com::Transform> &transforms,
                Eng::sparse_array<Com::Velocity> &velocities) {
            movementSystem(r, deltaTimeClock, transforms, velocities);
        });
    reg.add_system<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::Drawable>>(
        [&deltaTimeClock](Eng::registry &r,
                Eng::sparse_array<Com::AnimatedSprite> &animatedSprites,
                Eng::sparse_array<Com::Drawable> &drawables) {
            animationSystem(r, deltaTimeClock, animatedSprites, drawables);
        });
    reg.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::AnimatedSprite>>(
        [&window](Eng::registry &r,
                Eng::sparse_array<Com::Transform> const &transforms,
                Eng::sparse_array<Com::Drawable> &drawables,
                Eng::sparse_array<Com::AnimatedSprite> const &animatedSprites) {
            drawableSystem(r, window, transforms, drawables, animatedSprites);
        });
    // reset deltaTimeClock if needed in other systems
    reg.add_system<>([&deltaTimeClock](Eng::registry &r) {
        deltaTimeClock.restart();
    });
}
}  // namespace Rtype::Client
