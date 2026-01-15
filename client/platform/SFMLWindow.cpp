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

bool SFMLWindow::HasFocus() const {
    return window_.hasFocus();
}

Engine::Graphics::Vector2f SFMLWindow::MapPixelToCoords(
    const Engine::Graphics::Vector2i &pixel) const {
    sf::Vector2i sf_pixel(pixel.x, pixel.y);
    sf::Vector2f sf_coords = window_.mapPixelToCoords(sf_pixel);
    return Engine::Graphics::Vector2f(sf_coords.x, sf_coords.y);
}

sf::RenderWindow &SFMLWindow::GetNativeWindow() {
    return window_;
}

}  // namespace Rtype::Client::Platform
