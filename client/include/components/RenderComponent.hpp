#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct Drawable {
    std::string spritePath;
    int z_index = 0;
    float opacity = 1.0f;
    float rotation = 0.0f;
    sf::Color color = sf::Color::White;
    sf::Sprite sprite;
    sf::Texture texture;
    bool isLoaded = false;

    explicit Drawable(
        const std::string &spritePath, int z_index = 0, float opacity = 1.0f)
        : spritePath("assets/images/" + spritePath),
          z_index(z_index),
          texture(),
          opacity(opacity),
          sprite(texture),
          isLoaded(false),
          color(sf::Color::White) {}

    // Non-copyable to avoid accidental sprite/texture pointer mismatches
    Drawable(Drawable const &) = delete;
    Drawable &operator=(Drawable const &) = delete;

    // Move constructor: ensure sprite is rebound to the moved texture and
    // visual properties are preserved.
    Drawable(Drawable &&other) noexcept
        : spritePath(std::move(other.spritePath)),
          z_index(other.z_index),
          opacity(other.opacity),
          texture(std::move(other.texture)),
          sprite(),
          isLoaded(other.isLoaded) {
        // Rebind sprite to the moved texture and copy visual state
        sprite.setTexture(texture, true);
        sprite.setTextureRect(other.sprite.getTextureRect());
        sprite.setScale(other.sprite.getScale());
        sprite.setOrigin(other.sprite.getOrigin());
        sprite.setPosition(other.sprite.getPosition());
        sprite.setRotation(other.sprite.getRotation());
        sprite.setColor(other.sprite.getColor());
    }

    // Move assignment: similar to move ctor
    Drawable &operator=(Drawable &&other) noexcept {
        if (this == &other)
            return *this;
        spritePath = std::move(other.spritePath);
        z_index = other.z_index;
        opacity = other.opacity;
        texture = std::move(other.texture);
        isLoaded = other.isLoaded;

        sprite = sf::Sprite();
        sprite.setTexture(texture, true);
        sprite.setTextureRect(other.sprite.getTextureRect());
        sprite.setScale(other.sprite.getScale());
        sprite.setOrigin(other.sprite.getOrigin());
        sprite.setPosition(other.sprite.getPosition());
        sprite.setRotation(other.sprite.getRotation());
        sprite.setColor(other.sprite.getColor());

        return *this;
    }
};

struct Shader {
    std::string shaderPath;
    std::shared_ptr<sf::Shader> shader;
    bool isLoaded = false;
    std::map<std::string, float> uniforms_float = {};

    explicit Shader(const std::string &path,
        std::vector<std::pair<std::string, float>> uf = {})
        : shaderPath("assets/shaders/" + path),
          shader(nullptr),
          isLoaded(false) {
        for (const auto &[name, value] : uf)
            uniforms_float[name] = value;
    }
};

struct AnimatedSprite {
    struct Animation {
        std::string path;
        int frameWidth;
        int frameHeight;
        int totalFrames;
        int current_frame;
        float frameDuration;
        bool loop;
        sf::Vector2f first_frame_position = sf::Vector2f(0.0f, 0.0f);
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f);
        sf::Texture texture;
        sf::Sprite sprite;

        bool isLoaded = false;

        Animation()
            : path(""),
              frameWidth(0),
              frameHeight(0),
              totalFrames(0),
              current_frame(0),
              frameDuration(0.0f),
              loop(false),
              first_frame_position(0.0f, 0.0f),
              offset(0.0f, 0.0f),
              texture(),
              sprite(),
              isLoaded(false) {}

        Animation(const std::string &path, int frameWidth, int frameHeight,
            int totalFrames, float frameDuration, bool loop,
            sf::Vector2f first_frame_position = sf::Vector2f(0.0f, 0.0f),
            sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f))
            : path(path.empty() ? "" : "assets/images/" + path),
              frameWidth(frameWidth),
              frameHeight(frameHeight),
              totalFrames(totalFrames),
              current_frame(0),
              frameDuration(frameDuration),
              loop(loop),
              first_frame_position(first_frame_position),
              offset(offset),
              texture(),
              sprite(),
              isLoaded(false) {}
    };

    std::map<std::string, Animation> animations;
    std::string currentAnimation;
    std::vector<std::pair<std::string, int>> animationQueue;

    bool animated = true;
    float elapsedTime = 0.0f;

    AnimatedSprite(int frameWidth, int frameHeight, float frameDuration,
        bool loop = true,
        sf::Vector2f first_frame_position = sf::Vector2f(0.0f, 0.0f),
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
        sf::Vector2f first_frame_position = sf::Vector2f(0.0f, 0.0f),
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f));

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

    std::vector<std::string> GetAnimationNames() const;
};

/**
 * @brief Text component for rendering text with SFML.
 *
 * The text position, rotation, and scale are controlled by the Transform
 * component. This component only manages the text content, appearance,
 * font loading, per-entity opacity, and an optional local offset.
 */
struct Text {
    std::string content;
    std::string fontPath;
    unsigned int characterSize = 30;
    sf::Color color = sf::Color::White;
    float opacity = 1.0f;
    int z_index = 0;
    sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f);

    sf::Text text;
    sf::Font font;
    bool is_loaded = false;

    explicit Text(const std::string &fontPath, const std::string &content = "",
        unsigned int characterSize = 30, int z_index = 0,
        sf::Color color = sf::Color::White,
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f))
        : content(content),
          fontPath("assets/fonts/" + fontPath),
          characterSize(characterSize),
          color(color),
          opacity(1.0f),
          z_index(z_index),
          is_loaded(false),
          offset(offset) {}

    // Non-copyable to avoid accidental font pointer mismatches
    Text(Text const &) = delete;
    Text &operator=(Text const &) = delete;

    // Move constructor: rebind text to the moved font
    Text(Text &&other) noexcept;

    // Move assignment: similar to move ctor
    Text &operator=(Text &&other) noexcept;
};

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;     // restant
    float maxLifetime;  // durée de vie initiale
};

struct ParticleEmitter {
    bool active = true;
    float duration_active = -1.f;
    float duration_past = 0.0f;

    std::vector<Particle> particles;
    std::size_t maxParticles = 300;

    float emissionRate = 200.f;  // particules / seconde
    float emissionAccumulator = 0.f;

    sf::Color startColor = sf::Color(80, 80, 255, 255);  // bleu
    sf::Color endColor = sf::Color(80, 80, 255, 0);      // bleu transparent

    sf::Vector2f offset = {0.f, 0.f};  // offset local par rapport au Transform

    float particleLifetime = 1.0f;  // durée de vie des particules en secondes
    float particleSpeed = 50.f;     // vitesse initiale des particules
    sf::Vector2f direction = {0.f, -1.f};  // direction initiale des particules
    float spreadAngle = 30.f;              // angle de dispersion en degrés
    float gravity = 0.f;                   // accélération verticale
    float emissionRadius = 0.f;            // rayon d'apparition des particules
    float start_size = 1.0f;               // taille de début des particules
    float end_size = 1.0f;                 // taille de fin des particules
    int z_index = 0;                       // layer de rendu des particules

    bool emitting = true;  // Does the Particle emit particle

    // pour le rendu
    sf::VertexArray vertices = sf::VertexArray(sf::Points);

    ParticleEmitter(float emissionRate = 200.f, std::size_t maxParticles = 300,
        sf::Color startColor = sf::Color(80, 80, 255, 255),
        sf::Color endColor = sf::Color(80, 80, 255, 0),
        sf::Vector2f offset = {0.f, 0.f}, bool active = true,
        float particleLifetime = 1.0f, float particleSpeed = 50.f,
        sf::Vector2f direction = {0.f, -1.f}, float spreadAngle = 30.f,
        float gravity = 0.f, float emissionRadius = 0.f,
        float start_size = 1.0f, float end_size = 1.0f, float duration = -1.f,
        int z_index = 0)
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
