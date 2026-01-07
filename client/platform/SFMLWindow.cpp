/**
 * @file SFMLWindow.cpp
 * @brief Implementation of SFMLWindow.
 */

#include "platform/SFMLWindow.hpp"

#include <string>

#include <SFML/Window/VideoMode.hpp>

namespace Rtype::Client::Platform {

SFMLWindow::SFMLWindow(
    unsigned int width, unsigned int height, const std::string &title)
    : window_(sf::VideoMode({width, height}), title) {}

bool SFMLWindow::IsOpen() const {
    return window_.isOpen();
}

void SFMLWindow::Close() {
    window_.close();
}

Engine::Graphics::Vector2i SFMLWindow::GetSize() const {
    sf::Vector2u size = window_.getSize();
    return Engine::Graphics::Vector2i(
        static_cast<int>(size.x), static_cast<int>(size.y));
}

void SFMLWindow::SetTitle(const std::string &title) {
    window_.setTitle(title);
}

void SFMLWindow::Display() {
    window_.display();
}

sf::RenderWindow &SFMLWindow::GetNativeWindow() {
    return window_;
}

}  // namespace Rtype::Client::Platform
