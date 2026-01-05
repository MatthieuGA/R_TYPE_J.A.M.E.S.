#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "input/Action.hpp"

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
 * @brief Poll input state and populate the Inputs components.
 *
 * This system uses the InputManager to query logical actions
 * rather than physical keys, enabling backend-agnostic input handling.
 *
 * @param reg Engine registry (unused)
 * @param input_manager Reference to the input manager for action queries
 * @param inputs Sparse array of Inputs components to update
 */
void InputSystem(Eng::registry &reg,
    Engine::Input::InputManager &input_manager,
    Eng::sparse_array<Com::Inputs> &inputs) {
    if (!input_manager.HasFocus())
        return;
    for (auto &&[i, input] : make_indexed_zipper(inputs)) {
        input.last_shoot_state = input.shoot;

        // Use logical actions via GetAxis helper
        input.horizontal = input_manager.GetAxis(
            Engine::Input::Action::MoveLeft, Engine::Input::Action::MoveRight);
        input.vertical = input_manager.GetAxis(
            Engine::Input::Action::MoveUp, Engine::Input::Action::MoveDown);
        input.shoot =
            input_manager.IsActionActive(Engine::Input::Action::Shoot);
    }
}
}  // namespace Rtype::Client
