#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <graphics/Types.hpp>

namespace Rtype::Client::Component {

/**
 * @brief Drawable component for sprite rendering.
 *
 * Stores a texture resource ID (loaded via video backend) and rendering
 * properties. No SFML types - completely backend-agnostic.
 */
struct Drawable {
    std::string texture_id;   // Unique ID for texture resource
    std::string sprite_path;  // File path (for loading)
    int z_index = 0;
    float opacity = 1.0f;
    Engine::Graphics::Color color = Engine::Graphics::Color::White;
    Engine::Graphics::IntRect
        texture_rect;  // Source rect in texture (for sprite sheets)
    Engine::Graphics::Vector2f origin{0.0f, 0.0f};  // Sprite origin
    Engine::Graphics::Vector2f scale{1.0f, 1.0f};   // Sprite scale
    float rotation = 0.0f;                          // Rotation in degrees
    bool is_loaded = false;

    explicit Drawable(
        const std::string &sprite_path, int z_index = 0, float opacity = 1.0f)
        : sprite_path("assets/images/" + sprite_path),
          texture_id(sprite_path),  // Use sprite_path as texture ID
          z_index(z_index),
          opacity(opacity),
          color(Engine::Graphics::Color::White),
          texture_rect(),
          origin(0.0f, 0.0f),
          scale(1.0f, 1.0f),
          rotation(0.0f),
          is_loaded(false) {}

    // Movable and copyable (no resource ownership issues)
    Drawable(Drawable const &) = default;
    Drawable &operator=(Drawable const &) = default;
    Drawable(Drawable &&) noexcept = default;
    Drawable &operator=(Drawable &&) noexcept = default;
};

/**
 * @brief Shader component for shader effects.
 *
 * Stores a shader resource ID and uniform parameters.
 */
struct Shader {
    std::string shader_id;    // Unique ID for shader resource
    std::string shader_path;  // File path (for loading)
    bool is_loaded = false;
    std::map<std::string, float> uniforms_float = {};

    explicit Shader(const std::string &path,
        std::vector<std::pair<std::string, float>> uf = {})
        : shader_path("assets/shaders/" + path),
          shader_id(path),  // Use path as shader ID
          is_loaded(false) {
        for (const auto &[name, value] : uf)
            uniforms_float[name] = value;
    }
};

/**
 * @brief Animated sprite component for frame-based animation.
 *
 * Works with Drawable component to animate sprite sheets.
 */
struct AnimatedSprite {
    bool animated = true;
    int frame_width;
    int frame_height;
    int total_frames;
    int current_frame = 0;
    float frame_duration = 0.1f;
    bool loop = true;
    Engine::Graphics::Vector2f first_frame_position{0.0f, 0.0f};
    float elapsed_time = 0.0f;

    AnimatedSprite(int frame_width, int frame_height, float frame_duration,
        bool loop = true,
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f),
        int total_frames = 0)
        : frame_width(frame_width),
          frame_height(frame_height),
          total_frames(total_frames),
          current_frame(0),
          frame_duration(frame_duration),
          loop(loop),
          elapsed_time(0.0f),
          animated(true),
          first_frame_position(first_frame_position) {}

    AnimatedSprite(int frame_width, int frame_height, int current_frame)
        : frame_width(frame_width),
          frame_height(frame_height),
          total_frames(0),
          current_frame(current_frame),
          frame_duration(0.1f),
          loop(true),
          elapsed_time(0.0f),
          animated(false) {}
};

/**
 * @brief Text component for rendering text.
 *
 * Stores a font resource ID and text rendering properties.
 * No SFML types - completely backend-agnostic.
 */
struct Text {
    std::string content;
    std::string font_id;    // Unique ID for font resource
    std::string font_path;  // File path (for loading)
    unsigned int character_size = 30;
    Engine::Graphics::Color color = Engine::Graphics::Color::White;
    float opacity = 1.0f;
    int z_index = 0;
    Engine::Graphics::Vector2f offset{0.0f, 0.0f};
    bool is_loaded = false;

    explicit Text(const std::string &font_path,
        const std::string &content = "", unsigned int character_size = 30,
        int z_index = 0,
        Engine::Graphics::Color color = Engine::Graphics::Color::White,
        Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
            0.0f, 0.0f))
        : content(content),
          font_path("assets/fonts/" + font_path),
          font_id(font_path),  // Use font_path as font ID
          character_size(character_size),
          color(color),
          opacity(1.0f),
          z_index(z_index),
          offset(offset),
          is_loaded(false) {}

    // Movable and copyable (no resource ownership issues)
    Text(Text const &) = default;
    Text &operator=(Text const &) = default;
    Text(Text &&) noexcept = default;
    Text &operator=(Text &&) noexcept = default;
};

}  // namespace Rtype::Client::Component
