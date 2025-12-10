#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Updates the particle emitter, emitting new particles and updating
 * existing ones.
 *
 * @param emitter The particle emitter to update.
 * @param transform The transform component of the parent entity.
 * @param dt Delta time since the last frame.
 */
void updateEmitter(
    Com::ParticleEmitter &emitter, const Com::Transform &transform, float dt) {
    // Update emission accumulator
    emitter.emissionAccumulator += emitter.emissionRate * dt;

    // Emit new particles
    while (emitter.emissionAccumulator >= 1.0f && emitter.emitting &&
           emitter.particles.size() < emitter.maxParticles) {
        emitter.emissionAccumulator -= 1.0f;

        Component::Particle newParticle;
        newParticle.position = sf::Vector2f(
            transform.x + emitter.offset.x, transform.y + emitter.offset.y);

        // Calculate particle velocity with spread angle
        std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> spread_dist(
            -emitter.spreadAngle, emitter.spreadAngle);
        std::uniform_real_distribution<float> radius_dist(
            0.0f, emitter.emissionRadius);
        std::uniform_real_distribution<float> radius_angle_dist(
            0.0f, 2.0f * 3.14159f);

        // Apply emission radius offset
        float radius_offset_angle = radius_angle_dist(rng);
        float radius_offset_dist = radius_dist(rng);
        newParticle.position.x +=
            std::cos(radius_offset_angle) * radius_offset_dist;
        newParticle.position.y +=
            std::sin(radius_offset_angle) * radius_offset_dist;

        // Direction angle in radians
        float base_angle =
            std::atan2(emitter.direction.y, emitter.direction.x);
        // Add random spread
        // Convert degrees to radians
        float angle_variation = spread_dist(rng) * 3.14159f / 180.0f;
        float final_angle = base_angle + angle_variation;

        // Calculate velocity based on final angle and speed
        float cos_angle = std::cos(final_angle);
        float sin_angle = std::sin(final_angle);
        newParticle.velocity = sf::Vector2f(cos_angle * emitter.particleSpeed,
            sin_angle * emitter.particleSpeed);

        newParticle.maxLifetime = emitter.particleLifetime;
        newParticle.lifetime = emitter.particleLifetime;
        emitter.particles.push_back(newParticle);
    }

    // Update existing particles
    for (auto it = emitter.particles.begin(); it != emitter.particles.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f) {
            it = emitter.particles.erase(it);
        } else {
            // Apply velocity
            it->position += it->velocity * dt;
            // Apply gravity
            it->velocity.y += emitter.gravity * dt;
            ++it;
        }
    }
}

/**
 * @brief Draws particles from the emitter to the vertex array.
 *
 * Updates the vertex array with the current particles, applying color
 * based on particle lifetime. Each particle is drawn as a small square size
 *
 * @param emitter The particle emitter to draw.
 * @param game_world The game world containing the render window.
 */
void drawEmitter(Com::ParticleEmitter &emitter, GameWorld &game_world) {
    emitter.vertices.clear();
    emitter.vertices.setPrimitiveType(sf::Quads);

    for (const auto &particle : emitter.particles) {
        float lifeRatio = particle.lifetime / particle.maxLifetime;
        sf::Color color = sf::Color(
            static_cast<sf::Uint8>(
                emitter.endColor.r +
                (emitter.startColor.r - emitter.endColor.r) * lifeRatio),
            static_cast<sf::Uint8>(
                emitter.endColor.g +
                (emitter.startColor.g - emitter.endColor.g) * lifeRatio),
            static_cast<sf::Uint8>(
                emitter.endColor.b +
                (emitter.startColor.b - emitter.endColor.b) * lifeRatio),
            static_cast<sf::Uint8>(
                emitter.endColor.a +
                (emitter.startColor.a - emitter.endColor.a) * lifeRatio));

        // Lerp size from start to end
        float size = emitter.end_size +
                     (emitter.start_size - emitter.end_size) * lifeRatio;
        float half_size = size * 0.5f;
        sf::Vector2f pos = particle.position;

        // Top-left
        sf::Vertex v1;
        v1.position = sf::Vector2f(pos.x - half_size, pos.y - half_size);
        v1.color = color;
        emitter.vertices.append(v1);

        // Top-right
        sf::Vertex v2;
        v2.position = sf::Vector2f(pos.x + half_size, pos.y - half_size);
        v2.color = color;
        emitter.vertices.append(v2);

        // Bottom-right
        sf::Vertex v3;
        v3.position = sf::Vector2f(pos.x + half_size, pos.y + half_size);
        v3.color = color;
        emitter.vertices.append(v3);

        // Bottom-left
        sf::Vertex v4;
        v4.position = sf::Vector2f(pos.x - half_size, pos.y + half_size);
        v4.color = color;
        emitter.vertices.append(v4);
    }

    game_world.window_.draw(emitter.vertices);
}

/**
 * @brief Set the origin of a drawable based on its transform.
 *
 * Uses `GetOffsetFromTransform` to calculate the correct origin
 * based on the texture size and transform.
 *
 * @param drawable Drawable component to update
 * @param transform Transform used for offset calculation
 */
void SetDrawableOrigin(
    Com::Drawable &drawable, const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromTransform(transform,
        sf::Vector2f(static_cast<float>(drawable.texture.getSize().x),
            static_cast<float>(drawable.texture.getSize().y)));
    drawable.sprite.setOrigin(-origin);
}

/**
 * @brief Initialize a drawable that uses a static texture.
 *
 * Loads the texture, sets the origin based on the transform.
 *
 * @param drawable Drawable component to initialize
 * @param transform Transform for origin calculation
 */
void InitializeDrawable(
    Com::Drawable &drawable, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
                  << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableOrigin(drawable, transform);
    drawable.isLoaded = true;
}

/**
 * @brief Draw a sprite with optional shader.
 *
 * If a shader component is provided and loaded, it sets the "time"
 * uniform and draws the sprite with the shader. Otherwise, it draws
 * the sprite normally.
 *
 * @param game_world The game world containing the render window.
 * @param sprite The sprite to draw.
 * @param drawable The drawable component (for reference).
 * @param shaderCompOpt Optional shader component pointer.
 */
void DrawSprite(GameWorld &game_world, sf::Sprite &sprite,
    Com::Drawable *drawable, std::optional<Com::Shader *> shaderCompOpt) {
    if (shaderCompOpt.has_value() && (*shaderCompOpt)->isLoaded) {
        ((*shaderCompOpt)->shader)
            ->setUniform("time",
                game_world.total_time_clock_.getElapsedTime().asSeconds());
        game_world.window_.draw(
            sprite, sf::RenderStates((*shaderCompOpt)->shader.get()));
    } else {
        game_world.window_.draw(sprite);
    }
}

void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];

    std::optional<Com::Shader *> shaderCompOpt = std::nullopt;
    if (shaders.has(i))
        shaderCompOpt = &shaders[i].value();

    // Calculate world position with hierarchical rotation
    sf::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);

    // Apply animation offset if this entity has an AnimatedSprite component
    if (animated_sprites.has(i)) {
        auto *animation = animated_sprites[i]->GetCurrentAnimation();
        if (animation != nullptr) {
            world_position.x += animation->offset.x;
            world_position.y += animation->offset.y;
        }
    }

    drawable->sprite.setPosition(world_position);
    sf::Vector2f world_scale =
        CalculateCumulativeScale(transform.value(), transforms);
    drawable->sprite.setScale(world_scale);
    // Child rotation only: apply the entity's own rotation
    drawable->sprite.setRotation(transform->rotationDegrees);
    sf::Color color = drawable->color;
    color.a = static_cast<sf::Uint8>(drawable->opacity * 255);
    drawable->sprite.setColor(color);
    DrawSprite(game_world, drawable->sprite, &drawable.value(), shaderCompOpt);
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    Eng::sparse_array<Com::ParticleEmitter> &emitters) {
    struct RenderItem {
        int index;
        int z_index;
        bool is_particle;
    };

    std::vector<RenderItem> render_order;

    // Collect drawable entities
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        RenderItem item;
        item.index = i;
        item.z_index = drawable.z_index;
        item.is_particle = false;
        render_order.push_back(item);
    }

    // Collect particle emitters
    for (auto &&[i, transform, emitter] :
        make_indexed_zipper(transforms, emitters)) {
        if (emitter.active) {
            RenderItem item;
            item.index = i;
            item.z_index = emitter.z_index;
            item.is_particle = true;
            render_order.push_back(item);
        }
    }

    // Sort by z_index (unified rendering order)
    std::sort(render_order.begin(), render_order.end(),
        [](const RenderItem &a, const RenderItem &b) {
            return a.z_index < b.z_index;
        });

    // Render in sorted order
    for (const auto &item : render_order) {
        if (item.is_particle) {
            auto &transform = transforms[item.index];
            auto &emitter = emitters[item.index];

            // Handle duration logic
            if (emitter->duration_active != -1.0f) {
                emitter->duration_past += game_world.last_delta_;
                if (emitter->duration_past >= emitter->duration_active) {
                    emitter->emitting = false;
                    emitter->duration_past = 0.0f;
                }
            }

            updateEmitter(
                emitter.value(), transform.value(), game_world.last_delta_);
            drawEmitter(emitter.value(), game_world);
        } else {
            RenderOneEntity(transforms, drawables, shaders, animated_sprites,
                game_world, item.index);
        }
    }
}
}  // namespace Rtype::Client
