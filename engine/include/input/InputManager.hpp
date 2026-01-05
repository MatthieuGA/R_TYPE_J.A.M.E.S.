/**
 * @file InputManager.hpp
 * @brief Manages input mappings and provides logical action queries.
 *
 * The InputManager is the central point for input handling. It:
 * - Holds an IInputBackend for raw input state
 * - Maps physical inputs (keys, buttons) to logical actions
 * - Provides action queries that game code should use
 *
 * Game code must use IsActionActive() instead of checking physical keys.
 */

#ifndef ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_
#define ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_

#include <array>
#include <memory>
#include <vector>

#include "input/Action.hpp"
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
 * @brief Manages input backend and action mappings.
 *
 * This class bridges the gap between physical inputs and logical actions.
 * Game systems query actions through this class, never physical inputs.
 */
class InputManager {
 public:
    static constexpr size_t kMaxBindingsPerAction = 4;

    /**
     * @brief Construct with an input backend.
     *
     * @param backend The input backend to use for raw input state.
     *                Ownership is transferred to the InputManager.
     */
    explicit InputManager(std::unique_ptr<IInputBackend> backend);

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
    bool IsActionActive(Action action) const;

    /**
     * @brief Get the axis value for a pair of opposing actions.
     *
     * Returns a value in the range [-1, 1] based on which actions are active.
     * Useful for movement axes.
     *
     * @param negative The action representing the negative direction
     * @param positive The action representing the positive direction
     * @return float The axis value: -1 (negative), 0 (neither/both), +1
     * (positive)
     */
    float GetAxis(Action negative, Action positive) const;

    // =========================================================================
    // Focus State
    // =========================================================================

    /**
     * @brief Check if the window has focus.
     *
     * @return true if the window has keyboard focus
     */
    bool HasFocus() const;

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
    bool IsMouseButtonPressed(MouseButton button) const;

    /**
     * @brief Get the mouse position relative to the window.
     *
     * @return MousePosition The current mouse position
     */
    MousePosition GetMousePosition() const;

    // =========================================================================
    // Binding Management
    // =========================================================================

    /**
     * @brief Bind a key to an action.
     *
     * @param action The action to bind to
     * @param key The key to bind
     */
    void BindKey(Action action, Key key);

    /**
     * @brief Bind a mouse button to an action.
     *
     * @param action The action to bind to
     * @param button The mouse button to bind
     */
    void BindMouseButton(Action action, MouseButton button);

    /**
     * @brief Clear all bindings for an action.
     *
     * @param action The action to clear bindings for
     */
    void ClearBindings(Action action);

    /**
     * @brief Set up default key bindings for the game.
     *
     * This configures the standard QZSD + Space layout.
     */
    void SetupDefaultBindings();

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
    std::array<std::vector<InputBinding>, static_cast<size_t>(Action::Count)>
        bindings_;
};

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_INPUTMANAGER_HPP_
