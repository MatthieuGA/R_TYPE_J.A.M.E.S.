#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "graphics/Types.hpp"

namespace Rtype::Client::Component {

/**
 * @brief Drawable metadata used by the rendering backend.
 *
 * Holds path and appearance properties. No SFML types here.
 */
struct Drawable {
    std::string spritePath;
    std::string texture_path;
    int z_index = 0;
    float opacity = 1.0f;
    float rotation_degrees = 0.0f;
    Engine::Graphics::Color color = Engine::Graphics::Color::White;
    Engine::Graphics::Vector2f scale = Engine::Graphics::Vector2f(1.0f, 1.0f);
    Engine::Graphics::IntRect current_rect = Engine::Graphics::IntRect();
    bool is_loaded = false;

    explicit Drawable(
        const std::string &spritePath_input, int z = 0, float op = 1.0f)
        : spritePath("assets/images/" + spritePath_input),
          texture_path("assets/images/" + spritePath_input),
          z_index(z),
          opacity(op) {}
};

/**
 * @brief Shader metadata for backend-managed shader.
 */
struct Shader {
    std::string shader_path;
    bool is_loaded = false;
    std::map<std::string, float> uniforms_float = {};

    explicit Shader(const std::string &path,
        std::vector<std::pair<std::string, float>> uf = {})
        : shader_path("assets/shaders/" + path), is_loaded(false) {
        for (const auto &[name, value] : uf)
            uniforms_float[name] = value;
    }
};

/**
 * @brief Animated sprite definition without SFML types.
 */
struct AnimatedSprite {
    struct Animation {
        std::string path;
        std::string texture_path;
        int frameWidth;
        int frameHeight;
        int totalFrames;
        int current_frame;
        float frameDuration;
        bool loop;
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f);
        Engine::Graphics::Vector2f offset =
            Engine::Graphics::Vector2f(0.0f, 0.0f);
        Engine::Graphics::IntRect current_rect = Engine::Graphics::IntRect();
        bool is_loaded = false;

        Animation()
            : path(""),
              texture_path(""),
              frameWidth(0),
              frameHeight(0),
              totalFrames(0),
              current_frame(0),
              frameDuration(0.0f),
              loop(false),
              first_frame_position(0.0f, 0.0f),
              offset(0.0f, 0.0f),
              current_rect(),
              is_loaded(false) {}

        Animation(const std::string &p, int fw, int fh, int tf, float fd,
            bool lp,
            Engine::Graphics::Vector2f ffp = Engine::Graphics::Vector2f(
                0.0f, 0.0f),
            Engine::Graphics::Vector2f of = Engine::Graphics::Vector2f(
                0.0f, 0.0f))
            : path(p.empty() ? "" : "assets/images/" + p),
              texture_path(p),
              frameWidth(fw),
              frameHeight(fh),
              totalFrames(tf),
              current_frame(0),
              frameDuration(fd),
              loop(lp),
              first_frame_position(ffp),
              offset(of),
              current_rect(),
              is_loaded(false) {}
    };

    std::map<std::string, Animation> animations;
    std::string currentAnimation;
    std::vector<std::pair<std::string, int>> animationQueue;

    bool animated = true;
    float elapsedTime = 0.0f;

    AnimatedSprite(int frameWidth, int frameHeight, float frameDuration,
        bool loop = true,
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f),
        int totalFrames = 0);

    AnimatedSprite(int frameWidth, int frameHeight, int current_frame);

    /**
     * @brief Add a new animation.
     */
    void AddAnimation(const std::string &name, const std::string &path,
        int frameWidth, int frameHeight, int totalFrames, float frameDuration,
        bool loop = true,
        Engine::Graphics::Vector2f first_frame_position =
            Engine::Graphics::Vector2f(0.0f, 0.0f),
        Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
            0.0f, 0.0f));

    bool SetCurrentAnimation(
        const std::string &name, bool reset = true, bool push_to_queue = true);

    Animation *GetCurrentAnimation();
    const Animation *GetCurrentAnimation() const;
    std::vector<std::string> GetAnimationNames() const;
};

/**
 * @brief Text metadata without SFML types.
 */
struct Text {
    std::string content;
    std::string fontPath;
    std::string font_path;
    unsigned int characterSize = 30;
    Engine::Graphics::Color color = Engine::Graphics::Color::White;
    float opacity = 1.0f;
    int z_index = 0;
    float rotation_degrees = 0.0f;
    Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(0.0f, 0.0f);
    bool is_loaded = false;

    explicit Text(const std::string &fontPath, const std::string &content = "",
        unsigned int characterSize = 30, int z_index = 0,
        Engine::Graphics::Color color = Engine::Graphics::Color::White,
        Engine::Graphics::Vector2f offset = Engine::Graphics::Vector2f(
            0.0f, 0.0f))
        : content(content),
          fontPath("assets/fonts/" + fontPath),
          font_path(fontPath),
          characterSize(characterSize),
          color(color),
          opacity(1.0f),
          z_index(z_index),
          is_loaded(false),
          offset(offset) {}
};

struct Particle {
    Engine::Graphics::Vector2f position;
    Engine::Graphics::Vector2f velocity;
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

    Engine::Graphics::Color startColor =
        Engine::Graphics::Color(80, 80, 255, 255);  // bleu
    Engine::Graphics::Color endColor =
        Engine::Graphics::Color(80, 80, 255, 0);  // bleu transparent

    Engine::Graphics::Vector2f offset = {
        0.f, 0.f};  // offset local par rapport au Transform

    float particleLifetime = 1.0f;  // durée de vie des particules en secondes
    float particleSpeed = 50.f;     // vitesse initiale des particules
    Engine::Graphics::Vector2f direction = {
        0.f, -1.f};              // direction initiale des particules
    float spreadAngle = 30.f;    // angle de dispersion en degrés
    float gravity = 0.f;         // accélération verticale
    float emissionRadius = 0.f;  // rayon d'apparition des particules
    float start_size = 1.0f;     // taille de début des particules
    float end_size = 1.0f;       // taille de fin des particules
    int z_index = 0;             // layer de rendu des particules

    bool emitting = true;  // Does the Particle emit particle

    ParticleEmitter(float emissionRate = 200.f, std::size_t maxParticles = 300,
        Engine::Graphics::Color startColor = Engine::Graphics::Color(
            80, 80, 255, 255),
        Engine::Graphics::Color endColor = Engine::Graphics::Color(
            80, 80, 255, 0),
        Engine::Graphics::Vector2f offset = {0.f, 0.f}, bool active = true,
        float particleLifetime = 1.0f, float particleSpeed = 50.f,
        Engine::Graphics::Vector2f direction = {0.f, -1.f},
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
