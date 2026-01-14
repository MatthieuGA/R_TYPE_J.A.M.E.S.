/**
 * @file GameAction.hpp
 * @brief Game-specific logical actions for R-Type.
 *
 * This file defines the game-specific actions that map to physical inputs.
 * The action definitions live in the GAME layer, not the engine, because
 * different games will have different actions.
 *
 * The engine provides a generic templated InputManager that works with
 * any action enum type.
 */

#ifndef CLIENT_INCLUDE_GAME_GAMEACTION_HPP_
#define CLIENT_INCLUDE_GAME_GAMEACTION_HPP_

#include <cstdint>

namespace Game {

/**
 * @brief R-Type game-specific logical actions.
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

}  // namespace Game

#endif  // CLIENT_INCLUDE_GAME_GAMEACTION_HPP_
