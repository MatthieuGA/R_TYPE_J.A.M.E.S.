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
 */
void drawEmitter(Com::ParticleEmitter &emitter) {
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
}

/**
 * @brief System that updates and renders all particle emitters.
 *
 * This system:
 * - Updates particle emission and lifetime
 * - Applies velocity and gravity to particles
 * - Renders particles using color interpolation based on lifetime
 *
 * @param reg The game registry (unused).
 * @param game_world The game world containing the render window and delta time
 * @param transforms Sparse array of transform components (read-only).
 * @param emitters Sparse array of particle emitter components (read-write).
 */
void ParticleSystemEmitterSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::ParticleEmitter> &emitters) {
    for (auto &&[i, transform, emitter] :
        make_indexed_zipper(transforms, emitters)) {
        if (!emitter.active)
            continue;
        if (emitter.duration_active != -1.0f) {
            emitter.duration_past += game_world.last_delta_;
            if (emitter.duration_past >= emitter.duration_active) {
                emitter.emitting = false;
                emitter.duration_past = 0.0f;
            }
        }

        updateEmitter(emitter, transform, game_world.last_delta_);
        drawEmitter(emitter);
        game_world.window_.draw(emitter.vertices);
    }
}

}  // namespace Rtype::Client
