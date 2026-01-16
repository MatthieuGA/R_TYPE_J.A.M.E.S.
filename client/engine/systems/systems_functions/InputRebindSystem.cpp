/**
 * @file InputRebindSystem.cpp
 * @brief System for handling input rebinding in Settings scene.
 *
 * Listens for key presses when rebinding is active and updates the input
 * manager.
 */

#include <iostream>
#include <vector>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "game/GameAction.hpp"
#include "game/InputRebindHelper.hpp"
#include "input/IInputBackend.hpp"
#include "input/Key.hpp"

namespace Rtype::Client {

/**
 * @brief System to handle input rebinding in Settings scene.
 *
 * Monitors raw key input when waiting_for_rebind_key_ is true,
 * and updates the input manager bindings accordingly.
 */
void InputRebindSystem(GameWorld &game_world) {
    if (!game_world.waiting_for_rebind_key_ ||
        !game_world.rebinding_action_.has_value()) {
        return;  // Not in rebind mode
    }

    if (!game_world.input_manager_) {
        return;
    }

    auto *backend = game_world.input_manager_->GetBackend();
    if (!backend) {
        return;
    }

    // Check all keys to see which one is pressed
    // We'll use a simple approach: check common keys
    std::vector<Engine::Input::Key> keys_to_check = {Engine::Input::Key::A,
        Engine::Input::Key::B, Engine::Input::Key::C, Engine::Input::Key::D,
        Engine::Input::Key::E, Engine::Input::Key::F, Engine::Input::Key::G,
        Engine::Input::Key::H, Engine::Input::Key::I, Engine::Input::Key::J,
        Engine::Input::Key::K, Engine::Input::Key::L, Engine::Input::Key::M,
        Engine::Input::Key::N, Engine::Input::Key::O, Engine::Input::Key::P,
        Engine::Input::Key::Q, Engine::Input::Key::R, Engine::Input::Key::S,
        Engine::Input::Key::T, Engine::Input::Key::U, Engine::Input::Key::V,
        Engine::Input::Key::W, Engine::Input::Key::X, Engine::Input::Key::Y,
        Engine::Input::Key::Z, Engine::Input::Key::Up,
        Engine::Input::Key::Down, Engine::Input::Key::Left,
        Engine::Input::Key::Right, Engine::Input::Key::Space,
        Engine::Input::Key::Enter, Engine::Input::Key::Escape,
        Engine::Input::Key::LShift, Engine::Input::Key::RShift,
        Engine::Input::Key::Backspace, Engine::Input::Key::Tab};

    for (Engine::Input::Key key : keys_to_check) {
        if (backend->IsKeyPressed(key) &&
            key != Engine::Input::Key::Escape) {  // Allow Escape to cancel
            // Found a pressed key! Rebind this action to it
            Game::Action action = game_world.rebinding_action_.value();

            // Clear old bindings for this action
            game_world.input_manager_->ClearBindings(action);

            // Add new binding
            game_world.input_manager_->BindKey(action, key);

            std::cout << "[InputRebind] Rebound "
                      << Game::GetActionName(action) << " to "
                      << Game::GetKeyName(key) << std::endl;

            // Exit rebind mode
            game_world.rebinding_action_ = std::nullopt;
            game_world.waiting_for_rebind_key_ = false;
            break;
        }
    }

    // Allow Escape to cancel rebinding
    if (backend->IsKeyPressed(Engine::Input::Key::Escape)) {
        std::cout << "[InputRebind] Rebinding cancelled" << std::endl;
        game_world.rebinding_action_ = std::nullopt;
        game_world.waiting_for_rebind_key_ = false;
    }
}

}  // namespace Rtype::Client
