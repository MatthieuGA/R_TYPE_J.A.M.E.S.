#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void InputSystem(Eng::registry &reg,
Eng::sparse_array<Com::Inputs> &inputs) {
    for (auto &&[i, input] : make_indexed_zipper(inputs)) {
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
