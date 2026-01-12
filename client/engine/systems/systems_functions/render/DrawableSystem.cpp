#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "graphics/IRenderContext.hpp"
#include "graphics/SFMLRenderContext.hpp"

namespace Rtype::Client {

/**
 * @brief Update particle emitter: emit new particles and update existing.
 */
void UpdateEmitter(
    Com::ParticleEmitter &emitter, const Com::Transform &transform, float dt) {
    emitter.emissionAccumulator += emitter.emissionRate * dt;

    while (emitter.emissionAccumulator >= 1.0f && emitter.emitting &&
           emitter.particles.size() < emitter.maxParticles) {
        emitter.emissionAccumulator -= 1.0f;

        Engine::Graphics::Vector2f offset_computed(
            emitter.offset.x * transform.scale.x,
            emitter.offset.y * transform.scale.y);

        Component::Particle new_particle;
        new_particle.position = Engine::Graphics::Vector2f(
            transform.x + offset_computed.x, transform.y + offset_computed.y);

        std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> spread_dist(
            -emitter.spreadAngle, emitter.spreadAngle);
        std::uniform_real_distribution<float> radius_dist(
            0.0f, emitter.emissionRadius);
        std::uniform_real_distribution<float> radius_angle_dist(
            0.0f, 2.0f * 3.14159f);

        float radius_offset_angle = radius_angle_dist(rng);
        float radius_offset_dist = radius_dist(rng);
        new_particle.position.x +=
            std::cos(radius_offset_angle) * radius_offset_dist;
        new_particle.position.y +=
            std::sin(radius_offset_angle) * radius_offset_dist;

        float base_angle =
            std::atan2(emitter.direction.y, emitter.direction.x);
        float angle_variation = spread_dist(rng) * 3.14159f / 180.0f;
        float final_angle = base_angle + angle_variation;

        float cos_angle = std::cos(final_angle);
        float sin_angle = std::sin(final_angle);
        new_particle.velocity =
            Engine::Graphics::Vector2f(cos_angle * emitter.particleSpeed,
                sin_angle * emitter.particleSpeed);

        new_particle.maxLifetime = emitter.particleLifetime;
        new_particle.lifetime = emitter.particleLifetime;
        emitter.particles.push_back(new_particle);
    }

    for (auto it = emitter.particles.begin(); it != emitter.particles.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f) {
            it = emitter.particles.erase(it);
        } else {
            it->position += it->velocity * dt;
            it->velocity.y += emitter.gravity * dt;
            ++it;
        }
    }
}

/**
 * @brief Draw particles via render context.
 */
void drawEmitter(Com::ParticleEmitter &emitter, GameWorld &game_world) {
    if (!game_world.GetRenderContext())
        return;

    std::vector<Engine::Graphics::VertexArray::Vertex> vertices;

    for (const auto &particle : emitter.particles) {
        float lifeRatio = particle.lifetime / particle.maxLifetime;
        Engine::Graphics::Color engine_color = Engine::Graphics::Color(
            static_cast<uint8_t>(
                emitter.endColor.r +
                (emitter.startColor.r - emitter.endColor.r) * lifeRatio),
            static_cast<uint8_t>(
                emitter.endColor.g +
                (emitter.startColor.g - emitter.endColor.g) * lifeRatio),
            static_cast<uint8_t>(
                emitter.endColor.b +
                (emitter.startColor.b - emitter.endColor.b) * lifeRatio),
            static_cast<uint8_t>(
                emitter.endColor.a +
                (emitter.startColor.a - emitter.endColor.a) * lifeRatio));

        float size = emitter.end_size +
                     (emitter.start_size - emitter.end_size) * lifeRatio;
        float half_size = size * 0.5f;

        vertices.push_back(
            {Engine::Graphics::Vector2f(particle.position.x - half_size,
                 particle.position.y - half_size),
                engine_color});
        vertices.push_back(
            {Engine::Graphics::Vector2f(particle.position.x + half_size,
                 particle.position.y - half_size),
                engine_color});
        vertices.push_back(
            {Engine::Graphics::Vector2f(particle.position.x + half_size,
                 particle.position.y + half_size),
                engine_color});
        vertices.push_back(
            {Engine::Graphics::Vector2f(particle.position.x - half_size,
                 particle.position.y + half_size),
                engine_color});
    }

    if (!vertices.empty()) {
        Engine::Graphics::VertexArray va{vertices.data(), vertices.size(),
            Engine::Graphics::VertexArray::Quads};
        game_world.GetRenderContext()->DrawVertexArray(va);
    }
}

/**
 * @brief Render a single drawable entity via IRenderContext.
 */
void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    Eng::sparse_array<Com::Shader> const &shaders, GameWorld &game_world,
    int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];

    if (!game_world.GetRenderContext())
        return;

    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);

    if (animated_sprites.has(i)) {
        auto *animation = animated_sprites[i]->GetCurrentAnimation();
        if (animation != nullptr) {
            world_position.x += animation->offset.x;
            world_position.y += animation->offset.y;
        }
    }

    Engine::Graphics::Vector2f world_scale =
        CalculateCumulativeScale(transform.value(), transforms);

    Engine::Graphics::Color engine_color = drawable->color;
    engine_color.a = static_cast<uint8_t>(drawable->opacity * 255);

    // Calculate origin using OriginTool for proper sprite centering
    // GetOffsetFromTransform returns negative offsets; negate to get positive
    // for SFML
    Engine::Graphics::Vector2f origin(0.0f, 0.0f);
    Engine::Graphics::Vector2f texture_size(0.0f, 0.0f);

    // Get texture size for origin calculation
    if (drawable->current_rect.width > 0 &&
        drawable->current_rect.height > 0) {
        // Animation frame or cropped sprite - use rect size
        texture_size = Engine::Graphics::Vector2f(
            static_cast<float>(drawable->current_rect.width),
            static_cast<float>(drawable->current_rect.height));
    } else {
        // Static sprite - query backend for full texture size
        auto *sfml_context =
            dynamic_cast<Rtype::Client::Graphics::SFMLRenderContext *>(
                game_world.GetRenderContext());
        if (sfml_context) {
            texture_size =
                sfml_context->GetTextureSize(drawable->texture_path.c_str());
        }
    }

    if (texture_size.x > 0.0f && texture_size.y > 0.0f) {
        origin = GetOffsetFromTransform(transform.value(), texture_size);
        // Negate to convert from offset to origin (positive coordinates for
        // SFML)
        origin.x = -origin.x;
        origin.y = -origin.y;
    }

    Engine::Graphics::DrawableSprite sprite_data{
        drawable->texture_path.c_str(), world_position, world_scale,
        transform->rotationDegrees, engine_color, drawable->current_rect,
        origin};

    // Optional shader data
    Engine::Graphics::DrawableShader shader_data;
    const Engine::Graphics::DrawableShader *shader_ptr = nullptr;
    if (shaders.has(i) && shaders[i]->is_loaded &&
        !shaders[i]->shaderPath.empty()) {
        shader_data.shader_path = shaders[i]->shaderPath.c_str();
        shader_data.time_seconds =
            game_world.total_time_clock_.GetElapsedTime().AsSeconds();
        shader_data.float_uniforms.clear();
        shader_data.float_uniforms.reserve(shaders[i]->uniforms_float.size());
        for (const auto &[name, value] : shaders[i]->uniforms_float) {
            shader_data.float_uniforms.push_back({name.c_str(), value});
        }
        shader_ptr = &shader_data;
    }

    game_world.GetRenderContext()->DrawSprite(sprite_data, shader_ptr);
}

/**
 * @brief System to render all drawable entities and particle emitters.
 */
void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> const &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    Eng::sparse_array<Com::ParticleEmitter> &emitters) {
    struct RenderItem {
        size_t index;
        int z_index;
        bool is_particle;
    };

    std::vector<RenderItem> render_order;

    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        RenderItem item{i, drawable.z_index, false};
        render_order.push_back(item);
    }

    for (auto &&[i, transform, emitter] :
        make_indexed_zipper(transforms, emitters)) {
        if (emitter.active) {
            UpdateEmitter(emitter, transform, game_world.last_delta_);
            RenderItem item{i, emitter.z_index, true};
            render_order.push_back(item);
        }
    }

    std::sort(render_order.begin(), render_order.end(),
        [](const RenderItem &a, const RenderItem &b) {
            return a.z_index < b.z_index;
        });

    for (const auto &item : render_order) {
        if (item.is_particle) {
            if (emitters.has(item.index)) {
                drawEmitter(emitters[item.index].value(), game_world);
            }
        } else {
            RenderOneEntity(transforms, drawables, animated_sprites, shaders,
                game_world, item.index);
        }
    }
}

}  // namespace Rtype::Client
