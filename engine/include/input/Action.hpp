/**
 * @file Action.hpp
 * @brief Logical game actions for the Engine input abstraction layer.
 *
 * This file defines backend-agnostic logical actions that game code
 * should use instead of physical input checks. The mapping from
 * physical inputs (keys, buttons) to these actions is handled by
 * the InputManager.
 *
 * Game code must NEVER check physical keys directly - only logical actions.
 */

#ifndef ENGINE_INCLUDE_INPUT_ACTION_HPP_
#define ENGINE_INCLUDE_INPUT_ACTION_HPP_

#include <cstdint>

namespace Engine {
namespace Input {

/**
 * @brief Logical game actions.
 *
 * These represent abstract game intentions, not physical inputs.
 * Multiple physical inputs can map to the same action.
 * The mapping is configurable through InputManager.
 */
enum class Action : uint8_t {
    // Movement actions
    MoveUp = 0,  ///< Move upward (typically Z or Up arrow)
    MoveDown,    ///< Move downward (typically S or Down arrow)
    MoveLeft,    ///< Move left (typically Q or Left arrow)
    MoveRight,   ///< Move right (typically D or Right arrow)

    // Combat actions
    Shoot,        ///< Primary fire (typically Space)
    ChargeShoot,  ///< Charge shot (typically hold Space)

    // UI actions
    Confirm,  ///< Confirm selection (typically Enter or Space)
    Cancel,   ///< Cancel/back (typically Escape)
    Pause,    ///< Pause game (typically Escape or P)

    // Menu navigation
    MenuUp,     ///< Navigate menu up
    MenuDown,   ///< Navigate menu down
    MenuLeft,   ///< Navigate menu left
    MenuRight,  ///< Navigate menu right

    Count  ///< Keep last -- total number of actions
};

/**
 * @brief Get a human-readable name for an action.
 *
 * @param action The action to get the name for
 * @return const char* The name of the action
 */
inline const char *GetActionName(Action action) {
    switch (action) {
        case Action::MoveUp:
            return "MoveUp";
        case Action::MoveDown:
            return "MoveDown";
        case Action::MoveLeft:
            return "MoveLeft";
        case Action::MoveRight:
            return "MoveRight";
        case Action::Shoot:
            return "Shoot";
        case Action::ChargeShoot:
            return "ChargeShoot";
        case Action::Confirm:
            return "Confirm";
        case Action::Cancel:
            return "Cancel";
        case Action::Pause:
            return "Pause";
        case Action::MenuUp:
            return "MenuUp";
        case Action::MenuDown:
            return "MenuDown";
        case Action::MenuLeft:
            return "MenuLeft";
        case Action::MenuRight:
            return "MenuRight";
        default:
            return "Unknown";
    }
}

}  // namespace Input
}  // namespace Engine

#endif  // ENGINE_INCLUDE_INPUT_ACTION_HPP_
