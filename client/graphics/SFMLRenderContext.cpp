/**
 * @file SFMLRenderContext.cpp
 * @brief Implementation of SFML render context.
 */

#include "graphics/SFMLRenderContext.hpp"

#include <iostream>
#include <memory>
#include <string>

#include <SFML/Graphics.hpp>

namespace Rtype::Client::Graphics {

SFMLRenderContext::SFMLRenderContext(sf::RenderWindow &window)
    : window_(window) {}

sf::Texture *SFMLRenderContext::GetOrLoadTexture(const char *path) {
    std::string path_str(path);

    // Check cache
    if (texture_cache_.find(path_str) != texture_cache_.end()) {
        return texture_cache_[path_str].get();
    }

    // Load texture
    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(path_str)) {
        std::cerr << "ERROR: Failed to load texture from " << path << "\n";
        return nullptr;
    }

    texture_cache_[path_str] = texture;
    return texture.get();
}

sf::Font *SFMLRenderContext::GetOrLoadFont(const char *path) {
    std::string path_str(path);

    // Check cache
    if (font_cache_.find(path_str) != font_cache_.end()) {
        return font_cache_[path_str].get();
    }

    // Load font
    auto font = std::make_shared<sf::Font>();
    if (!font->loadFromFile(path_str)) {
        std::cerr << "ERROR: Failed to load font from " << path << "\n";
        return nullptr;
    }

    font_cache_[path_str] = font;
    return font.get();
}

sf::Color SFMLRenderContext::ToSFMLColor(
    const Engine::Graphics::Color &color) {
    return sf::Color(color.r, color.g, color.b, color.a);
}

void SFMLRenderContext::DrawSprite(
    const Engine::Graphics::DrawableSprite &sprite, void *shader_ptr) {
    // Load texture
    sf::Texture *texture = GetOrLoadTexture(sprite.texture_path);
    if (!texture) {
        return;  // Texture failed to load
    }

    // Create sprite
    sf::Sprite sfml_sprite(*texture);
    sfml_sprite.setPosition(sprite.position.x, sprite.position.y);
    sfml_sprite.setScale(sprite.scale.x, sprite.scale.y);
    sfml_sprite.setRotation(sprite.rotation_degrees);
    sfml_sprite.setColor(ToSFMLColor(sprite.color));

    // Draw with optional shader
    if (shader_ptr != nullptr) {
        auto *shader = static_cast<sf::Shader *>(shader_ptr);
        window_.draw(sfml_sprite, sf::RenderStates(shader));
    } else {
        window_.draw(sfml_sprite);
    }
}

void SFMLRenderContext::DrawText(const Engine::Graphics::DrawableText &text) {
    // Load font
    sf::Font *font = GetOrLoadFont(text.font_path);
    if (!font) {
        return;  // Font failed to load
    }

    // Create text
    sf::Text sfml_text;
    sfml_text.setFont(*font);
    sfml_text.setString(text.text);
    sfml_text.setCharacterSize(text.size);
    sfml_text.setPosition(text.position.x, text.position.y);
    sfml_text.setFillColor(ToSFMLColor(text.color));

    // Draw
    window_.draw(sfml_text);
}

void SFMLRenderContext::DrawRectangle(
    const Engine::Graphics::DrawableRectangle &rect) {
    sf::RectangleShape rectangle(sf::Vector2f(rect.size.x, rect.size.y));
    rectangle.setPosition(rect.position.x, rect.position.y);
    rectangle.setFillColor(ToSFMLColor(rect.color));
    window_.draw(rectangle);
}

void SFMLRenderContext::DrawVertexArray(
    const Engine::Graphics::VertexArray &vertices) {
    // Create SFML vertex array
    sf::VertexArray sfml_vertices(
        static_cast<sf::PrimitiveType>(vertices.primitive_type),
        vertices.vertex_count);

    // Copy vertices
    for (size_t i = 0; i < vertices.vertex_count; ++i) {
        const auto &src = vertices.vertices[i];
        sfml_vertices[i].position =
            sf::Vector2f(src.position.x, src.position.y);
        sfml_vertices[i].color = ToSFMLColor(src.color);
    }

    // Draw
    window_.draw(sfml_vertices);
}

}  // namespace Rtype::Client::Graphics
