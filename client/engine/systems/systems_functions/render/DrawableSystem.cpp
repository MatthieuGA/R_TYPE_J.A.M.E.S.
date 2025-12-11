#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
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
void UpdateEmitter(
    Com::ParticleEmitter &emitter, const Com::Transform &transform, float dt) {
    // Update emission accumulator
    emitter.emissionAccumulator += emitter.emissionRate * dt;

    // Emit new particles
    while (emitter.emissionAccumulator >= 1.0f && emitter.emitting &&
           emitter.particles.size() < emitter.maxParticles) {
        emitter.emissionAccumulator -= 1.0f;

        Engine::Graphics::Vector2f offset_computed(
            emitter.offset.x * transform.scale.x,
            emitter.offset.y * transform.scale.y);

        Component::Particle newParticle;
        newParticle.position = Engine::Graphics::Vector2f(
            transform.x + offset_computed.x, transform.y + offset_computed.y);

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
        // Add random spread (convert degrees to radians)
        float angle_variation = spread_dist(rng) * 3.14159f / 180.0f;
        float final_angle = base_angle + angle_variation;

        // Calculate velocity based on final angle and speed
        float cos_angle = std::cos(final_angle);
        float sin_angle = std::sin(final_angle);
        newParticle.velocity =
            Engine::Graphics::Vector2f(cos_angle * emitter.particleSpeed,
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
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            // Apply gravity
            it->velocity.y += emitter.gravity * dt;
            ++it;
        }
    }
}

/**
 * @brief Draws particles from the emitter using rendering engine.
 *
 * Renders particles using the high-level RenderParticles() API for efficient
 * batch rendering. Colors and sizes are interpolated based on particle
 * lifetime.
 *
 * @param emitter The particle emitter to draw.
 * @param game_world The game world containing the rendering engine.
 */
void DrawEmitter(Com::ParticleEmitter &emitter, GameWorld &game_world) {
    if (!game_world.rendering_engine_ || emitter.particles.empty()) {
        return;
    }

    // Prepare batch data for efficient rendering
    std::vector<Engine::Graphics::Vector2f> positions;
    std::vector<Engine::Graphics::Color> colors;
    std::vector<float> sizes;

    positions.reserve(emitter.particles.size());
    colors.reserve(emitter.particles.size());
    sizes.reserve(emitter.particles.size());

    for (const auto &particle : emitter.particles) {
        float lifeRatio = particle.lifetime / particle.maxLifetime;

        // Lerp color from start to end based on lifetime
        Engine::Graphics::Color color(
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

        // Lerp size from start to end
        float size = emitter.end_size +
                     (emitter.start_size - emitter.end_size) * lifeRatio;

        positions.push_back(particle.position);
        colors.push_back(color);
        sizes.push_back(size);
    }

    // Batch render all particles efficiently
    game_world.rendering_engine_->RenderParticles(
        positions, colors, sizes, emitter.z_index);
}

/**
 * @brief Initialize a drawable by loading its texture via rendering engine.
 *
 * @param drawable Drawable component to initialize
 * @param game_world Game world containing rendering engine
 */
void InitializeDrawable(Com::Drawable &drawable, GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        std::cerr << "[DrawableSystem] ERROR: rendering_engine is null!"
                  << std::endl;
        return;
    }

    std::cout << "[DrawableSystem] Loading texture: '" << drawable.sprite_path
              << "' with ID: '" << drawable.texture_id << "'" << std::endl;

    // Load texture via rendering engine
    bool loaded = game_world.rendering_engine_->LoadTexture(
        drawable.texture_id, drawable.sprite_path);

    if (!loaded) {
        std::cerr << "[DrawableSystem] ERROR: Failed to load texture: "
                  << drawable.sprite_path << " (ID: " << drawable.texture_id
                  << ")" << std::endl;
    } else {
        std::cout << "[DrawableSystem] Successfully loaded texture: "
                  << drawable.texture_id << std::endl;
    }

    drawable.is_loaded = loaded;
}

/**
 * @brief Render one drawable entity using rendering engine.
 *
 * Applies transform properties, animation offsets, and renders the sprite.
 *
 * @param transforms Sparse array of Transform components.
 * @param drawables Sparse array of Drawable components.
 * @param shaders Sparse array of Shader components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param game_world The game world containing the rendering engine.
 * @param i The index of the entity to render.
 */
void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];

    if (!game_world.rendering_engine_) {
        return;
    }

    // Calculate world position with hierarchical rotation
    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);

    // Apply animation offset if this entity has an AnimatedSprite component
    if (animated_sprites.has(i)) {
        auto *animation = animated_sprites[i]->GetCurrentAnimation();
        if (animation != nullptr) {
            world_position.x += animation->offset.x;
            world_position.y += animation->offset.y;
        }
    }

    float world_scale =
        CalculateCumulativeScale(transform.value(), transforms);

    // Calculate origin based on sprite size (texture_rect or full texture)
    Engine::Graphics::Vector2f sprite_size;
    if (drawable->texture_rect.width > 0 &&
        drawable->texture_rect.height > 0) {
        // Use texture_rect dimensions (for sprite sheets)
        sprite_size = Engine::Graphics::Vector2f(
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
    } else {
        // Use full texture size
        sprite_size =
            game_world.rendering_engine_->GetTextureSize(drawable->texture_id);
    }
    Engine::Graphics::Vector2f origin_offset =
        GetOffsetFromTransform(transform.value(), sprite_size);

    // Apply opacity to color
    Engine::Graphics::Color final_color = drawable->color;
    final_color.a = static_cast<uint8_t>(drawable->opacity * 255.0f);

    // Get texture rect if needed (for sprite sheets)
    Engine::Graphics::FloatRect *texture_rect_ptr = nullptr;
    Engine::Graphics::FloatRect texture_rect;
    if (drawable->texture_rect.width > 0 &&
        drawable->texture_rect.height > 0) {
        texture_rect = Engine::Graphics::FloatRect(
            static_cast<float>(drawable->texture_rect.left),
            static_cast<float>(drawable->texture_rect.top),
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
        texture_rect_ptr = &texture_rect;
    }

    // Check if this entity has a shader and apply uniforms before rendering
    const std::string *shader_id_ptr = nullptr;
    std::string shader_id_str;
    if (shaders.has(i) && shaders[i]->is_loaded) {
        shader_id_str = shaders[i]->shader_id;
        shader_id_ptr = &shader_id_str;

        // Set time uniform for wave effects
        float time = game_world.total_time_clock_.getElapsedTime().asSeconds();
        game_world.rendering_engine_->SetShaderParameter(
            shader_id_str, "time", time);

        // Set other shader uniforms from the shader component
        for (const auto &[uniform_name, uniform_value] :
            shaders[i]->uniforms_float) {
            game_world.rendering_engine_->SetShaderParameter(
                shader_id_str, uniform_name, uniform_value);
        }
    }

    // Use high-level RenderSprite method from RenderingEngine
    game_world.rendering_engine_->RenderSprite(drawable->texture_id,
        world_position, world_scale, transform->rotationDegrees,
        texture_rect_ptr, final_color, origin_offset, shader_id_ptr);
}

/**
 * @brief System to render all drawable entities and particle emitters.
 *
 * Collects all drawable entities and active particle emitters,
 * sorts them by z-index, and renders them in order.
 *
 * @param reg Engine registry.
 * @param game_world The game world containing the rendering engine.
 * @param transforms Sparse array of Transform components.
 * @param drawables Sparse array of Drawable components.
 * @param shaders Sparse array of Shader components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param emitters Sparse array of ParticleEmitter components.
 */
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
    static int frame_count = 0;
    bool debug = (frame_count++ % 60 == 0);

    // Initialize and collect drawable entities
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.is_loaded) {
            InitializeDrawable(drawable, game_world);
        }
        if (drawable.is_loaded) {
            RenderItem item;
            item.index = i;
            item.z_index = drawable.z_index;
            item.is_particle = false;
            render_order.push_back(item);

            if (debug && drawable.texture_id.find("r-typesheet2") !=
                             std::string::npos) {
                std::cout << "[DrawableSystem] Entity " << i
                          << " projectile ready to draw: pos=(" << transform.x
                          << "," << transform.y << "), z=" << drawable.z_index
                          << ", rect=(" << drawable.texture_rect.width << "x"
                          << drawable.texture_rect.height << ")" << std::endl;
            }
        }
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

            UpdateEmitter(
                emitter.value(), transform.value(), game_world.last_delta_);
            DrawEmitter(emitter.value(), game_world);
        } else {
            RenderOneEntity(transforms, drawables, shaders, animated_sprites,
                game_world, item.index);
        }
    }
}

}  // namespace Rtype::Client
