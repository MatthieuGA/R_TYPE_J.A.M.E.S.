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
 * Animation rendering is handled by the plugin backend.
 */
struct AnimatedSprite {
    struct Animation {
        std::string path;
        std::string texture_id;  // Unique ID for texture resource
        int frameWidth;
        int frameHeight;
        int totalFrames;
        int current_frame;
        float frameDuration;
        bool loop;
        Engine::Graphics::Vector2f first_frame_position{0.0f, 0.0f};
        Engine::Graphics::Vector2f offset{0.0f, 0.0f};

        bool isLoaded = false;

        Animation()
            : path(""),
              texture_id(""),
              frameWidth(0),
              frameHeight(0),
              totalFrames(0),
              current_frame(0),
              frameDuration(0.0f),
              loop(false),
              first_frame_position(0.0f, 0.0f),
              offset(0.0f, 0.0f),
              isLoaded(false) {}

        Animation(const std::string &path, int frameWidth, int frameHeight,
            int totalFrames, float frameDuration, bool loop,
            Engine::Graphics::Vector2f first_frame_position =
                Engine::Graphics::Vector2f(0.0f, 0.0f),
            Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
                0.0f, 0.0f))
            : path(path.empty() ? "" : "assets/images/" + path),
              texture_id(path),  // Use path as texture ID
              frameWidth(frameWidth),
              frameHeight(frameHeight),
              totalFrames(totalFrames),
              current_frame(0),
              frameDuration(frameDuration),
              loop(loop),
              first_frame_position(first_frame_position),
              offset(offset),
              isLoaded(false) {}
    };

    std::map<std::string, Animation> animations;
    std::string currentAnimation;
    std::vector<std::pair<std::string, int>> animationQueue;

    bool animated = true;
    float elapsedTime = 0.0f;

    AnimatedSprite(int frame_width, int frame_height, float frame_duration,
        bool loop = true,
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f),
        int totalFrames = 0);

    AnimatedSprite(int frameWidth, int frameHeight, int current_frame);

    /**
     * @brief Add a new animation to the animation map.
     *
     * @param name The name/key for this animation
     * @param path The path to the texture file (relative to assets/images/)
     * @param frameWidth Width of a single frame
     * @param frameHeight Height of a single frame
     * @param totalFrames Total number of frames in the animation
     * @param frameDuration Duration of each frame in seconds
     * @param loop Whether the animation should loop
     * @param first_frame_position Position of the first frame in the
     * spritesheet
     * @param offset Offset to apply to the sprite position when rendering
     */
    void AddAnimation(const std::string &name, const std::string &path,
        int frameWidth, int frameHeight, int totalFrames, float frameDuration,
        bool loop = true,
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f),
        Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
            0.0f, 0.0f));

    /**
     * @brief Change the current playing animation.
     *
     * @param name The name of the animation to play
     * @param reset If true, reset the animation to frame 0 and elapsed time to
     * 0
     * @param push_to_queue If true, store the currently playing animation to
     * resume later when interrupted
     * @return true if the animation exists and was changed, false otherwise
     */
    bool SetCurrentAnimation(
        const std::string &name, bool reset = true, bool push_to_queue = true);

    /**
     * @brief Get the current animation object.
     *
     * @return Pointer to the current Animation, or nullptr if not found
     */
    Animation *GetCurrentAnimation();

    /**
     * @brief Get the current animation object (const version).
     *
     * @return Const pointer to the current Animation, or nullptr if not found
     */
    const Animation *GetCurrentAnimation() const;
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

    // Non-copyable to avoid accidental font pointer mismatches
    Text(Text const &) = delete;
    Text &operator=(Text const &) = delete;

    // Move constructor: rebind text to the moved font
    Text(Text &&other) noexcept;

    // Move assignment: similar to move ctor
    Text &operator=(Text &&other) noexcept;
};

/**
 * @brief Particle data for particle systems.
 *
 * Backend-agnostic particle representation.
 */
struct Particle {
    Engine::Graphics::Vector2f position;
    Engine::Graphics::Vector2f velocity;
    float lifetime;     // remaining
    float maxLifetime;  // initial lifetime
};

/**
 * @brief Particle emitter component for particle effects.
 *
 * Particle rendering is handled by the plugin backend.
 * No SFML types - completely backend-agnostic.
 */
struct ParticleEmitter {
    bool active = true;
    float duration_active = -1.f;
    float duration_past = 0.0f;

    std::vector<Particle> particles;
    std::size_t maxParticles = 300;

    float emissionRate = 200.f;  // particles / second
    float emissionAccumulator = 0.f;

    Engine::Graphics::Color startColor =
        Engine::Graphics::Color(80, 80, 255, 255);  // blue
    Engine::Graphics::Color endColor =
        Engine::Graphics::Color(80, 80, 255, 0);  // transparent blue

    Engine::Graphics::Vector2f offset{
        0.f, 0.f};  // local offset from Transform

    float particleLifetime = 1.0f;  // particle lifetime in seconds
    float particleSpeed = 50.f;     // initial particle speed
    Engine::Graphics::Vector2f direction{0.f, -1.f};  // initial direction
    float spreadAngle = 30.f;    // spread angle in degrees
    float gravity = 0.f;         // vertical acceleration
    float emissionRadius = 0.f;  // emission radius for particles
    float start_size = 1.0f;     // particle start size
    float end_size = 1.0f;       // particle end size
    int z_index = 0;             // rendering layer

    bool emitting = true;  // Whether the emitter is emitting particles

    ParticleEmitter(float emissionRate = 200.f, std::size_t maxParticles = 300,
        Engine::Graphics::Color startColor = Engine::Graphics::Color(
            80, 80, 255, 255),
        Engine::Graphics::Color endColor = Engine::Graphics::Color(
            80, 80, 255, 0),
        Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
            0.f, 0.f),
        bool active = true, float particleLifetime = 1.0f,
        float particleSpeed = 50.f,
        Engine::Graphics::Vector2f direction = Engine::Graphics::Vector2f(
            0.f, -1.f),
        float spreadAngle = 30.f, float gravity = 0.f,
        float emissionRadius = 0.f, float start_size = 1.0f,
        float end_size = 1.0f, float duration = -1.f, int z_index = 0)
        : active(active),
          duration_active(duration),
          duration_past(0.0f),
          maxParticles(maxParticles),
          emissionRate(emissionRate),
          startColor(startColor),
          endColor(endColor),
          offset(offset),
          particleLifetime(particleLifetime),
          particleSpeed(particleSpeed),
          direction(direction),
          spreadAngle(spreadAngle),
          gravity(gravity),
          emissionRadius(emissionRadius),
          start_size(start_size),
          end_size(end_size),
          z_index(z_index) {}
};

}  // namespace Rtype::Client::Component
