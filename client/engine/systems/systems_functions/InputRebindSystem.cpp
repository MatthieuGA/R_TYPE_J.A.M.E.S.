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
#include "include/ColorsConst.hpp"
#include "include/components/RenderComponent.hpp"
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

    // Check for Escape first - this exits rebind mode
    if (backend->IsKeyPressed(Engine::Input::Key::Escape)) {
        std::cout << "[InputRebind] Rebinding finished (Escape pressed)"
                  << std::endl;
        // Reset button color to white
        if (game_world.rebinding_button_entity_.has_value()) {
            try {
                auto &text =
                    game_world.registry_.GetComponent<Component::Text>(
                        game_world.rebinding_button_entity_.value());
                text.color = WHITE_BLUE;
            } catch (...) {}
        }
        game_world.rebinding_action_ = std::nullopt;
        game_world.waiting_for_rebind_key_ = false;
        game_world.rebinding_button_entity_ = std::nullopt;
        return;
    }

    for (Engine::Input::Key key : keys_to_check) {
        if (backend->IsKeyPressed(key) && key != Engine::Input::Key::Escape) {
            // Found a pressed key! Add this binding to the action
            Game::Action action = game_world.rebinding_action_.value();

            // Check if this key is already bound to this action
            const auto &existing_bindings =
                game_world.input_manager_->GetBindings(action);
            bool already_bound = false;
            for (const auto &binding : existing_bindings) {
                if (binding.type == Engine::Input::InputBinding::Type::Key &&
                    binding.key == key) {
                    already_bound = true;
                    break;
                }
            }

            if (already_bound) {
                std::cout << "[InputRebind] Key " << Game::GetKeyName(key)
                          << " already bound to "
                          << Game::GetActionName(action) << ", skipping"
                          << std::endl;
                break;  // Don't add duplicate
            }

            // Add new binding (don't clear existing ones)
            game_world.input_manager_->BindKey(action, key);

            std::cout << "[InputRebind] Added binding for "
                      << Game::GetActionName(action) << ": "
                      << Game::GetKeyName(key) << " (press Escape to finish)"
                      << std::endl;

            // Save settings to disk after binding change
            game_world.SaveSettings();

            // Call callback to refresh UI icons
            if (game_world.on_binding_added_) {
                game_world.on_binding_added_(action);
            }

            // Don't exit rebind mode - wait for Escape
            // Small delay to prevent repeated binding of same key
            break;
        }
    }
}

}  // namespace Rtype::Client
