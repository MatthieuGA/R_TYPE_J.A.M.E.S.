/**
 * @file InputRebindHelper.hpp
 * @brief Helper utilities for input rebinding UI in Settings scene.
 *
 * Provides simple utilities for displaying key names and managing rebind
 * state.
 */

#ifndef CLIENT_INCLUDE_GAME_INPUTREBINDHELPER_HPP_
#define CLIENT_INCLUDE_GAME_INPUTREBINDHELPER_HPP_

#include <string>

#include "input/Key.hpp"

namespace Game {

/**
 * @brief Get human-readable name for a key.
 *
 * @param key The key to get the name for
 * @return std::string The name of the key
 */
inline std::string GetKeyName(Engine::Input::Key key) {
    switch (key) {
        case Engine::Input::Key::A:
            return "A";
        case Engine::Input::Key::B:
            return "B";
        case Engine::Input::Key::C:
            return "C";
        case Engine::Input::Key::D:
            return "D";
        case Engine::Input::Key::E:
            return "E";
        case Engine::Input::Key::F:
            return "F";
        case Engine::Input::Key::G:
            return "G";
        case Engine::Input::Key::H:
            return "H";
        case Engine::Input::Key::I:
            return "I";
        case Engine::Input::Key::J:
            return "J";
        case Engine::Input::Key::K:
            return "K";
        case Engine::Input::Key::L:
            return "L";
        case Engine::Input::Key::M:
            return "M";
        case Engine::Input::Key::N:
            return "N";
        case Engine::Input::Key::O:
            return "O";
        case Engine::Input::Key::P:
            return "P";
        case Engine::Input::Key::Q:
            return "Q";
        case Engine::Input::Key::R:
            return "R";
        case Engine::Input::Key::S:
            return "S";
        case Engine::Input::Key::T:
            return "T";
        case Engine::Input::Key::U:
            return "U";
        case Engine::Input::Key::V:
            return "V";
        case Engine::Input::Key::W:
            return "W";
        case Engine::Input::Key::X:
            return "X";
        case Engine::Input::Key::Y:
            return "Y";
        case Engine::Input::Key::Z:
            return "Z";
        case Engine::Input::Key::Up:
            return "Up";
        case Engine::Input::Key::Down:
            return "Down";
        case Engine::Input::Key::Left:
            return "Left";
        case Engine::Input::Key::Right:
            return "Right";
        case Engine::Input::Key::Space:
            return "Space";
        case Engine::Input::Key::Enter:
            return "Enter";
        case Engine::Input::Key::Escape:
            return "Escape";
        case Engine::Input::Key::LShift:
            return "LShift";
        case Engine::Input::Key::RShift:
            return "RShift";
        case Engine::Input::Key::Backspace:
            return "Backspace";
        case Engine::Input::Key::Tab:
            return "Tab";
        default:
            return "Unknown";
    }
}

/**
 * @brief Get the asset path for a keyboard key icon.
 *
 * @param key The key to get the asset path for
 * @return std::string The relative asset path (e.g.,
 * "keyboard/caracters/key_A.png")
 */
inline std::string GetKeyAssetPath(Engine::Input::Key key) {
    switch (key) {
        // Letters
        case Engine::Input::Key::A:
            return "keyboard/caracters/key_A.png";
        case Engine::Input::Key::B:
            return "keyboard/caracters/key_B.png";
        case Engine::Input::Key::C:
            return "keyboard/caracters/key_C.png";
        case Engine::Input::Key::D:
            return "keyboard/caracters/key_D.png";
        case Engine::Input::Key::E:
            return "keyboard/caracters/key_E.png";
        case Engine::Input::Key::F:
            return "keyboard/caracters/key_F.png";
        case Engine::Input::Key::G:
            return "keyboard/caracters/key_G.png";
        case Engine::Input::Key::H:
            return "keyboard/caracters/key_H.png";
        case Engine::Input::Key::I:
            return "keyboard/caracters/key_I.png";
        case Engine::Input::Key::J:
            return "keyboard/caracters/key_J.png";
        case Engine::Input::Key::K:
            return "keyboard/caracters/key_K.png";
        case Engine::Input::Key::L:
            return "keyboard/caracters/key_L.png";
        case Engine::Input::Key::M:
            return "keyboard/caracters/key_M.png";
        case Engine::Input::Key::N:
            return "keyboard/caracters/key_N.png";
        case Engine::Input::Key::O:
            return "keyboard/caracters/key_O.png";
        case Engine::Input::Key::P:
            return "keyboard/caracters/key_P.png";
        case Engine::Input::Key::Q:
            return "keyboard/caracters/key_Q.png";
        case Engine::Input::Key::R:
            return "keyboard/caracters/key_R.png";
        case Engine::Input::Key::S:
            return "keyboard/caracters/key_S.png";
        case Engine::Input::Key::T:
            return "keyboard/caracters/key_T.png";
        case Engine::Input::Key::U:
            return "keyboard/caracters/key_U.png";
        case Engine::Input::Key::V:
            return "keyboard/caracters/key_V.png";
        case Engine::Input::Key::W:
            return "keyboard/caracters/key_W.png";
        case Engine::Input::Key::X:
            return "keyboard/caracters/key_X.png";
        case Engine::Input::Key::Y:
            return "keyboard/caracters/key_Y.png";
        case Engine::Input::Key::Z:
            return "keyboard/caracters/key_Z.png";
        // Arrow keys
        case Engine::Input::Key::Up:
            return "keyboard/caracters/key_arrowup.png";
        case Engine::Input::Key::Down:
            return "keyboard/caracters/key_arrowdown.png";
        case Engine::Input::Key::Left:
            return "keyboard/caracters/key_arrowleft.png";
        case Engine::Input::Key::Right:
            return "keyboard/caracters/key_arrowright.png";
        // Special keys
        case Engine::Input::Key::Space:
            return "keyboard/special/key_space.png";
        case Engine::Input::Key::Enter:
            return "keyboard/special/key_enter.png";
        case Engine::Input::Key::Escape:
            return "keyboard/special/key_esc.png";
        case Engine::Input::Key::Tab:
            return "keyboard/special/key_tab.png";
        case Engine::Input::Key::Backspace:
            return "keyboard/special/key_back.png";
        case Engine::Input::Key::Delete:
            return "keyboard/special/key_del.png";
        case Engine::Input::Key::LShift:
        case Engine::Input::Key::RShift:
            return "keyboard/special/key_shift.png";
        case Engine::Input::Key::LControl:
        case Engine::Input::Key::RControl:
            return "keyboard/special/key_ctrl.png";
        case Engine::Input::Key::LAlt:
        case Engine::Input::Key::RAlt:
            return "keyboard/special/key_alt.png";
        // Function keys
        case Engine::Input::Key::F1:
            return "keyboard/caracters/key_F1.png";
        case Engine::Input::Key::F2:
            return "keyboard/caracters/key_F2.png";
        case Engine::Input::Key::F3:
            return "keyboard/caracters/key_F3.png";
        case Engine::Input::Key::F4:
            return "keyboard/caracters/key_F4.png";
        case Engine::Input::Key::F5:
            return "keyboard/caracters/key_F5.png";
        case Engine::Input::Key::F6:
            return "keyboard/caracters/key_F6.png";
        case Engine::Input::Key::F7:
            return "keyboard/caracters/key_F7.png";
        case Engine::Input::Key::F8:
            return "keyboard/caracters/key_F8.png";
        case Engine::Input::Key::F9:
            return "keyboard/caracters/key_F9.png";
        case Engine::Input::Key::F10:
            return "keyboard/caracters/key_F10.png";
        case Engine::Input::Key::F11:
            return "keyboard/caracters/key_F11.png";
        case Engine::Input::Key::F12:
            return "keyboard/caracters/key_F12.png";
        default:
            return "";  // No asset available
    }
}

}  // namespace Game

#endif  // CLIENT_INCLUDE_GAME_INPUTREBINDHELPER_HPP_
