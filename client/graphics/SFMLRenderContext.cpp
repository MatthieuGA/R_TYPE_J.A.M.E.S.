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

sf::Shader *SFMLRenderContext::GetOrLoadShader(
    const Engine::Graphics::DrawableShader &shader) {
    std::string path_str(shader.shader_path);

    // Check cache
    if (shader_cache_.find(path_str) != shader_cache_.end()) {
        return shader_cache_[path_str].get();
    }

    // Load shader (fragment)
    auto sf_shader = std::make_shared<sf::Shader>();
    if (!sf_shader->loadFromFile(path_str, sf::Shader::Type::Fragment)) {
        std::cerr << "ERROR: Failed to load shader from " << shader.shader_path
                  << "\n";
        return nullptr;
    }

    // Set static sampler
    sf_shader->setUniform("texture", sf::Shader::CurrentTexture);

    // Apply float uniforms
    for (const auto &uniform : shader.float_uniforms) {
        if (uniform.name != nullptr)
            sf_shader->setUniform(uniform.name, uniform.value);
    }

    shader_cache_[path_str] = sf_shader;
    return sf_shader.get();
}

void SFMLRenderContext::DrawSprite(
    const Engine::Graphics::DrawableSprite &sprite,
    const Engine::Graphics::DrawableShader *shader_ptr) {
    // Load texture
    sf::Texture *texture = GetOrLoadTexture(sprite.texture_path);
    if (!texture) {
        return;  // Texture failed to load
    }

    // Create sprite
    sf::Sprite sfml_sprite(*texture);

    // Apply texture rectangle for animation frames or cropping
    if (sprite.source_rect.width > 0 && sprite.source_rect.height > 0) {
        sfml_sprite.setTextureRect(
            sf::IntRect(sprite.source_rect.left, sprite.source_rect.top,
                sprite.source_rect.width, sprite.source_rect.height));
    }

    // Apply origin for centering and rotation
    sfml_sprite.setOrigin(sprite.origin.x, sprite.origin.y);

    // Apply transformation
    sfml_sprite.setPosition(sprite.position.x, sprite.position.y);
    sfml_sprite.setScale(sprite.scale.x, sprite.scale.y);
    sfml_sprite.setRotation(sprite.rotation_degrees);
    sfml_sprite.setColor(ToSFMLColor(sprite.color));

    // Draw with optional shader
    if (shader_ptr != nullptr) {
        sf::Shader *sf_shader = GetOrLoadShader(*shader_ptr);
        if (sf_shader != nullptr) {
            // Time uniform (per-draw)
            sf_shader->setUniform("time", shader_ptr->time_seconds);
            // Apply float uniforms each draw to ensure updates
            for (const auto &uniform : shader_ptr->float_uniforms) {
                if (uniform.name != nullptr)
                    sf_shader->setUniform(uniform.name, uniform.value);
            }
            window_.draw(sfml_sprite, sf::RenderStates(sf_shader));
        } else {
            window_.draw(sfml_sprite);
        }
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

    // Apply origin for centering
    sfml_text.setOrigin(text.origin.x, text.origin.y);

    // Apply transformation
    sfml_text.setPosition(text.position.x, text.position.y);
    sfml_text.setScale(text.scale.x, text.scale.y);
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

Engine::Graphics::Vector2f SFMLRenderContext::GetTextureSize(
    const char *path) {
    sf::Texture *texture = GetOrLoadTexture(path);
    if (!texture) {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }
    sf::Vector2u size = texture->getSize();
    return Engine::Graphics::Vector2f(
        static_cast<float>(size.x), static_cast<float>(size.y));
}

Engine::Graphics::Vector2f SFMLRenderContext::GetTextBounds(
    const char *font_path, const char *text, unsigned int size) {
    sf::Font *font = GetOrLoadFont(font_path);
    if (!font) {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }
    sf::Text sfml_text;
    sfml_text.setFont(*font);
    sfml_text.setString(text);
    sfml_text.setCharacterSize(size);
    sf::FloatRect bounds = sfml_text.getLocalBounds();
    return Engine::Graphics::Vector2f(bounds.width, bounds.height);
}

Engine::Graphics::Vector2i SFMLRenderContext::GetGridFrameSize(
    const char *texture_path, int grid_cols, int frame_width) {
    sf::Texture *texture = GetOrLoadTexture(texture_path);
    if (!texture) {
        return Engine::Graphics::Vector2i(0, 0);
    }
    sf::Vector2u texture_size = texture->getSize();
    // Calculate frame height based on grid layout
    // Total height / (texture_height / frame_width * grid_cols) = frame_height
    if (grid_cols <= 0 || frame_width <= 0) {
        return Engine::Graphics::Vector2i(frame_width, 0);
    }
    int frame_height = static_cast<int>(texture_size.y) /
                       ((static_cast<int>(texture_size.x) / frame_width));
    if (frame_height <= 0) {
        frame_height = static_cast<int>(texture_size.y);
    }
    return Engine::Graphics::Vector2i(frame_width, frame_height);
}

}  // namespace Rtype::Client::Graphics
