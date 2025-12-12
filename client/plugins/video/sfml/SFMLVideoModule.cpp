/**
 * @file SFMLVideoModule.cpp
 * @brief Implementation of SFML video backend.
 */

#include "SFMLVideoModule.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

// ===== Lifecycle =====

bool SFMLVideoModule::Initialize(
    unsigned int width, unsigned int height, const std::string &title) {
    try {
        window_ = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(width, height), title);
        window_->setFramerateLimit(60);

        // Set the view to match the window size exactly (0,0 top-left)
        sf::View view(sf::FloatRect(
            0, 0, static_cast<float>(width), static_cast<float>(height)));
        window_->setView(view);

        if (!window_->isOpen()) {
            std::cerr << "[SFMLVideoModule] ERROR: Window failed to open!"
                      << std::endl;
            return false;
        }

        std::cout << "[SFMLVideoModule] Initialized: " << width << "x"
                  << height << " - " << title << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "[SFMLVideoModule] Initialization failed: " << e.what()
                  << std::endl;
        return false;
    }
}

void SFMLVideoModule::Shutdown() {
    if (window_) {
        window_->close();
        window_.reset();
    }
    textures_.clear();
    fonts_.clear();
    shaders_.clear();
    std::cout << "[SFMLVideoModule] Shutdown complete" << std::endl;
}

void SFMLVideoModule::Update(float delta_time) {
    // Placeholder for per-frame updates if needed
    (void)delta_time;
}

// ===== Window Management =====

bool SFMLVideoModule::IsWindowOpen() const {
    return window_ && window_->isOpen();
}

void SFMLVideoModule::CloseWindow() {
    if (window_) {
        window_->close();
    }
}

Engine::Video::Vector2f SFMLVideoModule::GetWindowSize() const {
    if (!window_) {
        return {0.0f, 0.0f};
    }
    auto size = window_->getSize();
    return {static_cast<float>(size.x), static_cast<float>(size.y)};
}

void SFMLVideoModule::SetWindowTitle(const std::string &title) {
    if (window_) {
        window_->setTitle(title);
    }
}

// ===== Event Handling =====

bool SFMLVideoModule::PollEvent(Engine::Video::Event &event) {
    if (!window_) {
        return false;
    }

    sf::Event sf_event;
    if (!window_->pollEvent(sf_event)) {
        return false;
    }

    event.type = FromSFMLEventType(sf_event.type);

    switch (sf_event.type) {
        case sf::Event::Resized:
            event.data.width = sf_event.size.width;
            event.data.height = sf_event.size.height;
            break;
        case sf::Event::KeyPressed:
        case sf::Event::KeyReleased:
            event.data.key_code = static_cast<int>(sf_event.key.code);
            break;
        case sf::Event::MouseMoved:
            event.data.mouse_x = sf_event.mouseMove.x;
            event.data.mouse_y = sf_event.mouseMove.y;
            break;
        case sf::Event::MouseButtonPressed:
        case sf::Event::MouseButtonReleased:
            event.data.mouse_button =
                static_cast<int>(sf_event.mouseButton.button);
            event.data.mouse_x = sf_event.mouseButton.x;
            event.data.mouse_y = sf_event.mouseButton.y;
            break;
        case sf::Event::MouseWheelScrolled:
            event.data.mouse_wheel_delta = sf_event.mouseWheelScroll.delta;
            break;
        default:
            break;
    }

    return true;
}

// ===== Rendering =====

void SFMLVideoModule::Clear(const Engine::Video::Color &color) {
    if (window_) {
        window_->clear(ToSFMLColor(color));
    }
}

void SFMLVideoModule::Display() {
    if (window_) {
        window_->display();
    }
}

// ===== Texture Management =====

bool SFMLVideoModule::LoadTexture(
    const std::string &id, const std::string &path) {
    // Check if texture is already loaded
    if (textures_.find(id) != textures_.end()) {
        texture_ref_counts_[id]++;  // Increment reference count
        return true;  // Already loaded, no need to reload
    }

    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(path)) {
        std::cerr << "[SFMLVideoModule] Failed to load texture: " << path
                  << std::endl;
        return false;
    }
    textures_[id] = texture;
    texture_ref_counts_[id] = 1;  // Initialize reference count
    std::cout << "[SFMLVideoModule] Loaded texture: " << id << " from " << path
              << " (refs: 1)" << std::endl;
    return true;
}

const void *SFMLVideoModule::GetTexture(const std::string &id) const {
    auto it = textures_.find(id);
    if (it == textures_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Engine::Video::Vector2f SFMLVideoModule::GetTextureSize(
    const std::string &id) const {
    auto it = textures_.find(id);
    if (it == textures_.end()) {
        return {0.0f, 0.0f};
    }
    auto size = it->second->getSize();
    return {static_cast<float>(size.x), static_cast<float>(size.y)};
}

bool SFMLVideoModule::UnloadTexture(const std::string &id) {
    auto it = textures_.find(id);
    if (it == textures_.end()) {
        return false;  // Texture not found
    }

    // Decrement reference count
    auto ref_it = texture_ref_counts_.find(id);
    if (ref_it != texture_ref_counts_.end()) {
        ref_it->second--;
        
        // Only unload if no more references
        if (ref_it->second <= 0) {
            std::cout << "[SFMLVideoModule] Unloading texture: " << id << std::endl;
            textures_.erase(it);
            texture_ref_counts_.erase(ref_it);
            return true;
        } else {
            std::cout << "[SFMLVideoModule] Decremented texture ref: " << id
                      << " (refs: " << ref_it->second << ")" << std::endl;
            return false;  // Still has references
        }
    }

    // No ref count tracking (shouldn't happen), unload anyway
    textures_.erase(it);
    return true;
}

// ===== Font Management =====

bool SFMLVideoModule::LoadFont(
    const std::string &id, const std::string &path) {
    // Check if font is already loaded
    if (fonts_.find(id) != fonts_.end()) {
        return true;  // Already loaded, no need to reload
    }

    auto font = std::make_shared<sf::Font>();
    if (!font->loadFromFile(path)) {
        std::cerr << "[SFMLVideoModule] Failed to load font: " << path
                  << std::endl;
        return false;
    }
    fonts_[id] = font;
    std::cout << "[SFMLVideoModule] Loaded font: " << id << " from " << path
              << std::endl;
    return true;
}

const void *SFMLVideoModule::GetFont(const std::string &id) const {
    auto it = fonts_.find(id);
    if (it == fonts_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Engine::Video::FloatRect SFMLVideoModule::GetTextBounds(
    const std::string &text, const std::string &font_id,
    unsigned int character_size) const {
    auto it = fonts_.find(font_id);
    if (it == fonts_.end()) {
        return Engine::Video::FloatRect(0, 0, 0, 0);
    }

    sf::Text sf_text;
    sf_text.setFont(*it->second);
    sf_text.setString(text);
    sf_text.setCharacterSize(character_size);

    sf::FloatRect bounds = sf_text.getLocalBounds();
    return Engine::Video::FloatRect(
        bounds.left, bounds.top, bounds.width, bounds.height);
}

// ===== Sprite Drawing =====

void SFMLVideoModule::DrawSprite(const std::string &texture_id,
    const Engine::Video::Transform &transform,
    const Engine::Video::FloatRect *texture_rect,
    const Engine::Video::Color &color, const std::string *shader_id) {
    if (!window_) {
        return;
    }

    auto it = textures_.find(texture_id);
    if (it == textures_.end()) {
        return;
    }

    sf::Sprite sprite;
    sprite.setTexture(*it->second);

    if (texture_rect) {
        sprite.setTextureRect(sf::IntRect(static_cast<int>(texture_rect->left),
            static_cast<int>(texture_rect->top),
            static_cast<int>(texture_rect->width),
            static_cast<int>(texture_rect->height)));
    }

    sprite.setPosition(transform.position.x, transform.position.y);
    sprite.setRotation(transform.rotation);
    sprite.setScale(transform.scale.x, transform.scale.y);
    sprite.setOrigin(transform.origin.x, transform.origin.y);
    sprite.setColor(ToSFMLColor(color));

    // Apply shader if provided
    if (shader_id && !shader_id->empty()) {
        auto shader_it = shaders_.find(*shader_id);
        if (shader_it != shaders_.end()) {
            sf::RenderStates states;
            states.shader = shader_it->second.get();
            window_->draw(sprite, states);
            return;
        }
    }

    window_->draw(sprite);
}

// ===== Text Drawing =====

void SFMLVideoModule::DrawText(const std::string &text,
    const std::string &font_id, const Engine::Video::Transform &transform,
    unsigned int character_size, const Engine::Video::Color &color) {
    if (!window_) {
        return;
    }

    auto it = fonts_.find(font_id);
    if (it == fonts_.end()) {
        return;
    }

    sf::Text sf_text;
    sf_text.setFont(*it->second);
    sf_text.setString(text);
    sf_text.setCharacterSize(character_size);
    sf_text.setFillColor(ToSFMLColor(color));

    // Apply transform
    sf_text.setPosition(transform.position.x, transform.position.y);
    sf_text.setRotation(transform.rotation);
    sf_text.setScale(transform.scale.x, transform.scale.y);
    sf_text.setOrigin(transform.origin.x, transform.origin.y);

    window_->draw(sf_text);
}

// ===== Primitive Drawing =====

void SFMLVideoModule::DrawRectangle(const Engine::Video::FloatRect &rect,
    const Engine::Video::Color &color,
    const Engine::Video::Color *outline_color, float outline_thickness) {
    if (!window_) {
        return;
    }

    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition(rect.left, rect.top);
    shape.setFillColor(ToSFMLColor(color));

    if (outline_color && outline_thickness > 0.0f) {
        shape.setOutlineColor(ToSFMLColor(*outline_color));
        shape.setOutlineThickness(outline_thickness);
    }

    window_->draw(shape);
}

void SFMLVideoModule::DrawCircle(const Engine::Video::Vector2f &center,
    float radius, const Engine::Video::Color &color,
    const Engine::Video::Color *outline_color, float outline_thickness) {
    if (!window_) {
        return;
    }

    sf::CircleShape shape(radius);
    shape.setPosition(center.x - radius, center.y - radius);
    shape.setFillColor(ToSFMLColor(color));

    if (outline_color && outline_thickness > 0.0f) {
        shape.setOutlineColor(ToSFMLColor(*outline_color));
        shape.setOutlineThickness(outline_thickness);
    }

    window_->draw(shape);
}

// ===== Advanced =====

void SFMLVideoModule::DrawVertices(const Engine::Video::Vertex *vertices,
    size_t vertex_count, int primitive_type,
    const Engine::Video::RenderStates &states) {
    std::cout << "[DEBUG] SFMLVideoModule::DrawVertices called with " << vertex_count 
              << " vertices, type=" << primitive_type << std::endl;
    
    if (!window_ || !vertices || vertex_count == 0) {
        std::cout << "[DEBUG] DrawVertices: Early return (window=" << (window_ ? "valid" : "null")
                  << ", vertices=" << (vertices ? "valid" : "null")
                  << ", count=" << vertex_count << ")" << std::endl;
        return;
    }

    std::cout << "[DEBUG] DrawVertices: Converting vertices..." << std::endl;
    // Convert to SFML vertices
    std::vector<sf::Vertex> sf_vertices(vertex_count);
    for (size_t i = 0; i < vertex_count; ++i) {
        sf_vertices[i].position = {
            vertices[i].position.x, vertices[i].position.y};
        sf_vertices[i].color = ToSFMLColor(vertices[i].color);
        sf_vertices[i].texCoords = {
            vertices[i].tex_coords.x, vertices[i].tex_coords.y};
    }

    // Map primitive type
    sf::PrimitiveType sf_type = sf::Points;
    switch (primitive_type) {
        case 0:
            sf_type = sf::Points;
            break;
        case 1:
            sf_type = sf::Lines;
            break;
        case 2:
            sf_type = sf::Triangles;
            break;
        case 3:
            sf_type = sf::Quads;
            break;
        default:
            break;
    }

    std::cout << "[DEBUG] DrawVertices: Conversion complete, creating render states" << std::endl;
    
    // Create render states
    sf::RenderStates sf_states;
    if (states.texture) {
        sf_states.texture =
            reinterpret_cast<const sf::Texture *>(states.texture);
        std::cout << "[DEBUG] DrawVertices: Texture set" << std::endl;
    }
    if (states.shader) {
        sf_states.shader = reinterpret_cast<const sf::Shader *>(states.shader);
        std::cout << "[DEBUG] DrawVertices: Shader set" << std::endl;
    }

    std::cout << "[DEBUG] DrawVertices: Calling window_->draw with " << vertex_count 
              << " vertices, type=" << sf_type << std::endl;
    
    window_->draw(sf_vertices.data(), vertex_count, sf_type, sf_states);
    
    std::cout << "[DEBUG] DrawVertices: Draw completed successfully" << std::endl;
}

// ===== Shader Management =====

bool SFMLVideoModule::LoadShader(const std::string &id,
    const std::string &vertex_path, const std::string &fragment_path) {
    auto shader = std::make_shared<sf::Shader>();

    bool success = false;
    if (!vertex_path.empty() && !fragment_path.empty()) {
        success = shader->loadFromFile(vertex_path, fragment_path);
    } else if (!vertex_path.empty()) {
        success = shader->loadFromFile(vertex_path, sf::Shader::Vertex);
    } else if (!fragment_path.empty()) {
        success = shader->loadFromFile(fragment_path, sf::Shader::Fragment);
    }

    if (!success) {
        std::cerr << "[SFMLVideoModule] Failed to load shader: " << id
                  << std::endl;
        return false;
    }

    shaders_[id] = shader;
    std::cout << "[SFMLVideoModule] Loaded shader: " << id << std::endl;
    return true;
}

void SFMLVideoModule::SetShaderParameter(
    const std::string &shader_id, const std::string &name, float value) {
    auto it = shaders_.find(shader_id);
    if (it == shaders_.end()) {
        return;
    }
    it->second->setUniform(name, value);
}

// ===== Metadata =====

std::string SFMLVideoModule::GetModuleName() const {
    return "SFML Video Module";
}

// ===== Compatibility Bridge =====

void *SFMLVideoModule::GetNativeWindow() const {
    return static_cast<void *>(window_.get());
}

// ===== Helper Functions =====

sf::Color SFMLVideoModule::ToSFMLColor(
    const Engine::Video::Color &color) const {
    return sf::Color(color.r, color.g, color.b, color.a);
}

Engine::Video::EventType SFMLVideoModule::FromSFMLEventType(
    sf::Event::EventType type) const {
    switch (type) {
        case sf::Event::Closed:
            return Engine::Video::EventType::CLOSED;
        case sf::Event::Resized:
            return Engine::Video::EventType::RESIZED;
        case sf::Event::LostFocus:
            return Engine::Video::EventType::LOST_FOCUS;
        case sf::Event::GainedFocus:
            return Engine::Video::EventType::GAINED_FOCUS;
        case sf::Event::KeyPressed:
            return Engine::Video::EventType::KEY_PRESSED;
        case sf::Event::KeyReleased:
            return Engine::Video::EventType::KEY_RELEASED;
        case sf::Event::MouseMoved:
            return Engine::Video::EventType::MOUSE_MOVED;
        case sf::Event::MouseButtonPressed:
            return Engine::Video::EventType::MOUSE_BUTTON_PRESSED;
        case sf::Event::MouseButtonReleased:
            return Engine::Video::EventType::MOUSE_BUTTON_RELEASED;
        case sf::Event::MouseWheelScrolled:
            return Engine::Video::EventType::MOUSE_WHEEL_SCROLLED;
        default:
            // Return RESIZED for unhandled events (safer than CLOSED!)
            // This includes TextEntered, JoystickMoved, etc.
            return Engine::Video::EventType::RESIZED;
    }
}

// ===== Plugin Entry Point =====

extern "C" {
std::shared_ptr<Engine::Video::IVideoModule> entryPoint() {
    return std::make_shared<SFMLVideoModule>();
}
}
