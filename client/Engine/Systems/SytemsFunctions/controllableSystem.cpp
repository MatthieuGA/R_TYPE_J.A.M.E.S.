#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void ControllableSystem(Eng::registry &reg,
Eng::sparse_array<Com::Inputs> &inputs,
Eng::sparse_array<Com::Controllable> const &controllables,
Eng::sparse_array<Com::Velocity> &velocities) {
    for (auto &&[i, input, controllable, velocity] :
    make_indexed_zipper(inputs, controllables, velocities)) {
        if (!controllable.isControllable)
            continue;

        // Update velocity based on inputs
        const float speed = 300.0f;
        velocity.vx = input.horizontal * speed;
        velocity.vy = input.vertical * speed;
    }
}
}  // namespace Rtype::Client
