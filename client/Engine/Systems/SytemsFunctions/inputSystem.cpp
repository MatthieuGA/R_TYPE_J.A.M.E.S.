#include <SFML/Graphics.hpp>

#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Poll keyboard state and populate the Inputs components.
 *
 * This system resets each `Inputs` entry and then maps a fixed set of keys
 * to the input axes and shoot flag.
 *
 * @param reg Engine registry (unused)
 * @param inputs Sparse array of Inputs components to update
 */
void InputSystem(Eng::registry &reg, Eng::sparse_array<Com::Inputs> &inputs) {
    for (auto &&[i, input] : make_indexed_zipper(inputs)) {
        input.last_shoot_state = input.shoot;

        input.horizontal = 0.0f;
        input.vertical = 0.0f;
        input.shoot = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            input.horizontal -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            input.horizontal += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            input.vertical -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            input.vertical += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            input.shoot = true;
    }
}
}  // namespace Rtype::Client
