#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Convert Inputs component to RFC-compliant bitfield.
 *
 * According to RFC Section 6.1 (PLAYER_INPUT):
 * Bit 0: Up
 * Bit 1: Down
 * Bit 2: Left
 * Bit 3: Right
 * Bit 4: Shoot
 *
 * @param input The Inputs component to convert
 * @return uint8_t Bitfield representation of inputs
 */
uint8_t InputToBitfield(const Com::Inputs &input) {
    uint8_t bitfield = 0;

    if (input.vertical < 0.0f)  // Up
        bitfield |= (1 << 0);
    if (input.vertical > 0.0f)  // Down
        bitfield |= (1 << 1);
    if (input.horizontal < 0.0f)  // Left
        bitfield |= (1 << 2);
    if (input.horizontal > 0.0f)  // Right
        bitfield |= (1 << 3);
    if (input.shoot)  // Shoot
        bitfield |= (1 << 4);

    return bitfield;
}

/**
 * @brief Poll keyboard state and populate the Inputs components.
 *
 * This system resets each `Inputs` entry and then maps a fixed set of keys
 * to the input axes and shoot flag.
 *
 * @param reg Engine registry (unused)
 * @param inputs Sparse array of Inputs components to update
 */
void InputSystem(Eng::registry &reg, bool has_focus,
    Eng::sparse_array<Com::Inputs> &inputs) {
    if (!has_focus)
        return;
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
