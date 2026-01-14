/**
 * @file GameInputBindings.hpp
 * @brief Default input bindings for R-Type game actions.
 *
 * This file contains the game-specific setup for input bindings.
 * It maps physical inputs (keys, mouse buttons) to game actions.
 * This lives in the GAME layer because bindings are game-specific.
 */

#ifndef CLIENT_INCLUDE_GAME_GAMEINPUTBINDINGS_HPP_
#define CLIENT_INCLUDE_GAME_GAMEINPUTBINDINGS_HPP_

#include "game/GameAction.hpp"
#include "input/InputManager.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Game {

/**
 * @brief Set up default key bindings for R-Type.
 *
 * Configures the standard QZSD/WASD + Space layout for the game.
 *
 * @tparam ActionT The action enum type (Game::Action)
 * @param input_manager The input manager to configure
 */
template <typename ActionT>
void SetupDefaultBindings(
    Engine::Input::InputManager<ActionT> &input_manager) {
    using Key = Engine::Input::Key;
    using MouseButton = Engine::Input::MouseButton;

    // Clear all existing bindings
    input_manager.ClearAllBindings();

    // Movement - QZSD layout (French keyboard) + WASD + Arrow keys
    input_manager.BindKey(Action::MoveUp, Key::Z);
    input_manager.BindKey(Action::MoveUp, Key::W);
    input_manager.BindKey(Action::MoveUp, Key::Up);

    input_manager.BindKey(Action::MoveDown, Key::S);
    input_manager.BindKey(Action::MoveDown, Key::Down);

    input_manager.BindKey(Action::MoveLeft, Key::Q);
    input_manager.BindKey(Action::MoveLeft, Key::A);
    input_manager.BindKey(Action::MoveLeft, Key::Left);

    input_manager.BindKey(Action::MoveRight, Key::D);
    input_manager.BindKey(Action::MoveRight, Key::Right);

    // Combat
    input_manager.BindKey(Action::Shoot, Key::Space);
    input_manager.BindMouseButton(Action::Shoot, MouseButton::Left);

    input_manager.BindKey(Action::ChargeShoot, Key::LShift);
    input_manager.BindMouseButton(Action::ChargeShoot, MouseButton::Right);

    // UI / Menu
    input_manager.BindKey(Action::Confirm, Key::Enter);
    input_manager.BindKey(Action::Confirm, Key::Space);

    input_manager.BindKey(Action::Cancel, Key::Escape);
    input_manager.BindKey(Action::Cancel, Key::Backspace);

    input_manager.BindKey(Action::Pause, Key::Escape);
    input_manager.BindKey(Action::Pause, Key::P);

    // Menu navigation (same as movement for consistency)
    input_manager.BindKey(Action::MenuUp, Key::Z);
    input_manager.BindKey(Action::MenuUp, Key::W);
    input_manager.BindKey(Action::MenuUp, Key::Up);

    input_manager.BindKey(Action::MenuDown, Key::S);
    input_manager.BindKey(Action::MenuDown, Key::Down);

    input_manager.BindKey(Action::MenuLeft, Key::Q);
    input_manager.BindKey(Action::MenuLeft, Key::A);
    input_manager.BindKey(Action::MenuLeft, Key::Left);

    input_manager.BindKey(Action::MenuRight, Key::D);
    input_manager.BindKey(Action::MenuRight, Key::Right);
}

}  // namespace Game

#endif  // CLIENT_INCLUDE_GAME_GAMEINPUTBINDINGS_HPP_
