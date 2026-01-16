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

}  // namespace Game

#endif  // CLIENT_INCLUDE_GAME_INPUTREBINDHELPER_HPP_
