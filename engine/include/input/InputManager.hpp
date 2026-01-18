/**
 * @file InputManager.hpp
 * @brief Generic input manager template for mapping physical inputs to
 * actions.
 *
 * The InputManager is a game-agnostic component that:
 * - Holds an IInputBackend for raw input state
 * - Maps physical inputs (keys, buttons) to action identifiers
 * - Provides action queries that game code uses
 *
 * The action type is templated, allowing each game to define its own
 * action enum without modifying engine code.
 *
 * Game code must use IsActionActive() instead of checking physical keys.
 */

#ifndef ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_
#define ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "input/IInputBackend.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

namespace Engine {
namespace Input {

/**
 * @brief Represents a physical input binding.
 *
 * Can be a keyboard key or a mouse button.
 */
struct InputBinding {
    enum class Type : uint8_t {
        None = 0,
        Key,
        MouseButton
    };

    Type type = Type::None;

    union {
        Key key;
        MouseButton mouse_button;
    };

    /**
     * @brief Create an empty binding.
     */
    InputBinding() : type(Type::None), key(Key::Unknown) {}

    /**
     * @brief Create a key binding.
     */
    explicit InputBinding(Key k) : type(Type::Key), key(k) {}

    /**
     * @brief Create a mouse button binding.
     */
    explicit InputBinding(MouseButton mb)
        : type(Type::MouseButton), mouse_button(mb) {}
};

/**
 * @brief Generic input manager template.
 *
 * This class bridges the gap between physical inputs and logical actions.
 * Game systems query actions through this class, never physical inputs.
 *
 * @tparam ActionT An enum class representing game-specific actions.
 *                 Must have a 'Count' member indicating the total count.
 *                 Must be convertible to size_t via static_cast.
 */
template <typename ActionT>
class InputManager {
    static_assert(std::is_enum_v<ActionT>, "ActionT must be an enum type");

 public:
    using ActionType = ActionT;
    static constexpr size_t kMaxBindingsPerAction = 4;
    static constexpr size_t kActionCount = static_cast<size_t>(ActionT::Count);

    /**
     * @brief Construct with an input backend.
     *
     * @param backend The input backend to use for raw input state.
     *                Ownership is transferred to the InputManager.
     */
    explicit InputManager(std::unique_ptr<IInputBackend> backend)
        : backend_(std::move(backend)) {
        for (auto &binding_list : bindings_) {
            binding_list.reserve(kMaxBindingsPerAction);
        }
    }

    /**
     * @brief Default destructor.
     */
    ~InputManager() = default;

    // Non-copyable
    InputManager(const InputManager &) = delete;
    InputManager &operator=(const InputManager &) = delete;

    // Movable
    InputManager(InputManager &&) = default;
    InputManager &operator=(InputManager &&) = default;

    // =========================================================================
    // Action Queries (Game code uses these)
    // =========================================================================

    /**
     * @brief Check if a logical action is currently active.
     *
     * An action is active if any of its bound inputs are pressed.
     * This is the primary method game code should use.
     *
     * @param action The action to check
     * @return true if the action is currently active
     */
    bool IsActionActive(ActionT action) const {
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

    /**
     * @brief Get the axis value for a pair of opposing actions.
     *
     * Returns a value in the range [-1, 1] based on which actions are active.
     * Useful for movement axes.
     *
     * @param negative The action representing the negative direction
     * @param positive The action representing the positive direction
     * @return float The axis value: -1 (negative), 0 (neither/both), +1
     *         (positive)
     */
    float GetAxis(ActionT negative, ActionT positive) const {
        float result = 0.0f;

        if (IsActionActive(negative)) {
            result -= 1.0f;
        }
        if (IsActionActive(positive)) {
            result += 1.0f;
        }

        return result;
    }

    // =========================================================================
    // Focus State
    // =========================================================================

    /**
     * @brief Check if the window has focus.
     *
     * @return true if the window has keyboard focus
     */
    bool HasFocus() const {
        return backend_ && backend_->HasWindowFocus();
    }

    // =========================================================================
    // Mouse Queries (for UI systems)
    // =========================================================================

    /**
     * @brief Check if a mouse button is pressed.
     *
     * For UI systems that need raw mouse state.
     *
     * @param button The mouse button to check
     * @return true if pressed
     */
    bool IsMouseButtonPressed(MouseButton button) const {
        return backend_ && backend_->IsMouseButtonPressed(button);
    }

    /**
     * @brief Get the mouse position relative to the window.
     *
     * @return MousePosition The current mouse position
     */
    MousePosition GetMousePosition() const {
        if (backend_) {
            return backend_->GetMousePositionInWindow();
        }
        return {0, 0};
    }

    // =========================================================================
    // Binding Management
    // =========================================================================

    /**
     * @brief Bind a key to an action.
     *
     * @param action The action to bind to
     * @param key The key to bind
     */
    void BindKey(ActionT action, Key key) {
        const auto action_index = static_cast<size_t>(action);
        if (action_index >= bindings_.size()) {
            return;
        }

        auto &action_bindings = bindings_[action_index];
        if (action_bindings.size() < kMaxBindingsPerAction) {
            action_bindings.emplace_back(key);
        }
    }

    /**
     * @brief Bind a mouse button to an action.
     *
     * @param action The action to bind to
     * @param button The mouse button to bind
     */
    void BindMouseButton(ActionT action, MouseButton button) {
        const auto action_index = static_cast<size_t>(action);
        if (action_index >= bindings_.size()) {
            return;
        }

        auto &action_bindings = bindings_[action_index];
        if (action_bindings.size() < kMaxBindingsPerAction) {
            action_bindings.emplace_back(button);
        }
    }

    /**
     * @brief Clear all bindings for a specific action.
     *
     * @param action The action to clear bindings for
     */
    void ClearBindings(ActionT action) {
        const auto action_index = static_cast<size_t>(action);
        if (action_index < bindings_.size()) {
            bindings_[action_index].clear();
        }
    }

    /**
     * @brief Get all bindings for a specific action.
     *
     * @param action The action to get bindings for
     * @return const std::vector<InputBinding>& The list of bindings
     */
    const std::vector<InputBinding> &GetBindings(ActionT action) const {
        const auto action_index = static_cast<size_t>(action);
        static const std::vector<InputBinding> empty;
        if (action_index >= bindings_.size()) {
            return empty;
        }
        return bindings_[action_index];
    }

    /**
     * @brief Clear all bindings for all actions.
     */
    void ClearAllBindings() {
        for (auto &binding_list : bindings_) {
            binding_list.clear();
        }
    }

    // =========================================================================
    // Backend Access
    // =========================================================================

    /**
     * @brief Get the underlying input backend.
     *
     * @return IInputBackend* Pointer to the backend (never null)
     */
    IInputBackend *GetBackend() const {
        return backend_.get();
    }

 private:
    std::unique_ptr<IInputBackend> backend_;

    // Bindings: action index → list of input bindings
    std::array<std::vector<InputBinding>, kActionCount> bindings_;
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_
