/**
 * @file InputManager.cpp
 * @brief Implementation of the InputManager class.
 */

#include "input/InputManager.hpp"

#include <utility>

namespace Engine {
namespace Input {

InputManager::InputManager(std::unique_ptr<IInputBackend> backend)
    : backend_(std::move(backend)) {
    // Reserve space for bindings
    for (auto &binding_list : bindings_) {
        binding_list.reserve(kMaxBindingsPerAction);
    }
}

bool InputManager::IsActionActive(Action action) const {
    if (!backend_ || !backend_->HasWindowFocus()) {
        return false;
    }

    const auto action_index = static_cast<size_t>(action);
    if (action_index >= bindings_.size()) {
        return false;
    }

    const auto &action_bindings = bindings_[action_index];
    for (const auto &binding : action_bindings) {
        switch (binding.type) {
            case InputBinding::Type::Key:
                if (backend_->IsKeyPressed(binding.key)) {
                    return true;
                }
                break;
            case InputBinding::Type::MouseButton:
                if (backend_->IsMouseButtonPressed(binding.mouse_button)) {
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return false;
}

float InputManager::GetAxis(Action negative, Action positive) const {
    float result = 0.0f;

    if (IsActionActive(negative)) {
        result -= 1.0f;
    }
    if (IsActionActive(positive)) {
        result += 1.0f;
    }

    return result;
}

bool InputManager::HasFocus() const {
    return backend_ && backend_->HasWindowFocus();
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    return backend_ && backend_->IsMouseButtonPressed(button);
}

MousePosition InputManager::GetMousePosition() const {
    if (backend_) {
        return backend_->GetMousePositionInWindow();
    }
    return {0, 0};
}

void InputManager::BindKey(Action action, Key key) {
    const auto action_index = static_cast<size_t>(action);
    if (action_index >= bindings_.size()) {
        return;
    }

    auto &action_bindings = bindings_[action_index];
    if (action_bindings.size() < kMaxBindingsPerAction) {
        action_bindings.emplace_back(key);
    }
}

void InputManager::BindMouseButton(Action action, MouseButton button) {
    const auto action_index = static_cast<size_t>(action);
    if (action_index >= bindings_.size()) {
        return;
    }

    auto &action_bindings = bindings_[action_index];
    if (action_bindings.size() < kMaxBindingsPerAction) {
        action_bindings.emplace_back(button);
    }
}

void InputManager::ClearBindings(Action action) {
    const auto action_index = static_cast<size_t>(action);
    if (action_index < bindings_.size()) {
        bindings_[action_index].clear();
    }
}

void InputManager::SetupDefaultBindings() {
    // Clear all existing bindings
    for (auto &binding_list : bindings_) {
        binding_list.clear();
    }

    // Movement - QZSD layout (French keyboard) + Arrow keys
    BindKey(Action::MoveUp, Key::Z);
    BindKey(Action::MoveUp, Key::W);
    BindKey(Action::MoveUp, Key::Up);

    BindKey(Action::MoveDown, Key::S);
    BindKey(Action::MoveDown, Key::Down);

    BindKey(Action::MoveLeft, Key::Q);
    BindKey(Action::MoveLeft, Key::A);
    BindKey(Action::MoveLeft, Key::Left);

    BindKey(Action::MoveRight, Key::D);
    BindKey(Action::MoveRight, Key::Right);

    // Combat
    BindKey(Action::Shoot, Key::Space);
    BindMouseButton(Action::Shoot, MouseButton::Left);

    BindKey(Action::ChargeShoot, Key::LShift);
    BindMouseButton(Action::ChargeShoot, MouseButton::Right);

    // UI / Menu
    BindKey(Action::Confirm, Key::Enter);
    BindKey(Action::Confirm, Key::Space);

    BindKey(Action::Cancel, Key::Escape);
    BindKey(Action::Cancel, Key::Backspace);

    BindKey(Action::Pause, Key::Escape);
    BindKey(Action::Pause, Key::P);

    // Menu navigation (same as movement for consistency)
    BindKey(Action::MenuUp, Key::Z);
    BindKey(Action::MenuUp, Key::W);
    BindKey(Action::MenuUp, Key::Up);

    BindKey(Action::MenuDown, Key::S);
    BindKey(Action::MenuDown, Key::Down);

    BindKey(Action::MenuLeft, Key::Q);
    BindKey(Action::MenuLeft, Key::A);
    BindKey(Action::MenuLeft, Key::Left);

    BindKey(Action::MenuRight, Key::D);
    BindKey(Action::MenuRight, Key::Right);
}

}  // namespace Input
}  // namespace Engine
