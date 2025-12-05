#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct Drawable {
    std::string spritePath;
    int z_index = 0;
    float opacity = 1.0f;
    float rotation = 0.0f;
    sf::Sprite sprite;
    sf::Texture texture;
    bool isLoaded = false;

    explicit Drawable(const std::string& spritePath, int z_index = 0,
        float opacity = 1.0f)
        : spritePath("Assets/Images/" + spritePath), z_index(z_index),
        texture(), opacity(opacity), sprite(texture), isLoaded(false) {}

    // Non-copyable to avoid accidental sprite/texture pointer mismatches
    Drawable(Drawable const &) = delete;
    Drawable &operator=(Drawable const &) = delete;

    // Move constructor: ensure sprite is rebound to the moved texture and
    // visual properties are preserved.
    Drawable(Drawable &&other) noexcept
        : spritePath(std::move(other.spritePath)), z_index(other.z_index),
          opacity(other.opacity), texture(std::move(other.texture)), sprite(),
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

    explicit Shader(const std::string& path,
        std::vector<std::pair<std::string, float>> uf = {})
        : shaderPath("Assets/Shaders/" + path), shader(nullptr),
        isLoaded(false) {
        for (const auto& [name, value] : uf)
            uniforms_float[name] = value;
    }
};

struct AnimatedSprite {
    bool animated = true;
    int frameWidth;
    int frameHeight;
    int totalFrames;
    int currentFrame = 0;
    float frameDuration = 0.1f;
    bool loop = true;
    float elapsedTime = 0.0f;

    AnimatedSprite(int frameWidth, int frameHeight, float frameDuration,
        bool loop = true)
        : frameWidth(frameWidth), frameHeight(frameHeight),
        totalFrames(0), currentFrame(0), frameDuration(frameDuration),
        loop(loop), elapsedTime(0.0f), animated(true) {}
    AnimatedSprite(int frameWidth, int frameHeight, int current_frame)
        : frameWidth(frameWidth), frameHeight(frameHeight),
        totalFrames(0), currentFrame(current_frame), frameDuration(0.1f),
        loop(true), elapsedTime(0.0f), animated(false) {}
};

}  // namespace Rtype::Client::Component
