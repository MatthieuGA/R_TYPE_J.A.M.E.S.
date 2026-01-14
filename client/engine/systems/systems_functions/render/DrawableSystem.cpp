#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

inline sf::Vector2f ToSFML(const Engine::Graphics::Vector2f &v) {
    return sf::Vector2f(v.x, v.y);
}

inline Engine::Graphics::Vector2f ToEngine(const sf::Vector2f &v) {
    return Engine::Graphics::Vector2f(v.x, v.y);
}

inline sf::Color ToSFML(const Engine::Graphics::Color &c) {
    return sf::Color(c.r, c.g, c.b, c.a);
}

inline Engine::Graphics::Color ToEngine(const sf::Color &c) {
    return Engine::Graphics::Color(c.r, c.g, c.b, c.a);
}

inline sf::IntRect ToSFML(const Engine::Graphics::IntRect &r) {
    return sf::IntRect(r.left, r.top, r.width, r.height);
}

inline Engine::Graphics::IntRect ToEngine(const sf::IntRect &r) {
    return Engine::Graphics::IntRect(r.left, r.top, r.width, r.height);
}

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

        sf::Vector2f offset_computed =
            sf::Vector2f(emitter.offset.x * transform.scale.x,
                emitter.offset.y * transform.scale.y);

        Component::Particle newParticle;
        newParticle.position = sf::Vector2f(
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
        sf::Color color = ToSFML(engine_color);

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

    game_world.GetNativeWindow().draw(emitter.vertices);
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
    sf::Vector2f size_vec(static_cast<float>(drawable.texture.getSize().x),
        static_cast<float>(drawable.texture.getSize().y));
    sf::Vector2f origin =
        ToSFML(GetOffsetFromTransform(transform, ToEngine(size_vec)));
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
    if (shaderCompOpt.has_value() && (*shaderCompOpt)->isLoaded &&
        (*shaderCompOpt)->shader != nullptr) {
        ((*shaderCompOpt)->shader)
            ->setUniform("time",
                game_world.total_time_clock_.GetElapsedTime().AsSeconds());
        game_world.GetNativeWindow().draw(
            sprite, sf::RenderStates((*shaderCompOpt)->shader.get()));
    } else {
        game_world.GetNativeWindow().draw(sprite);
    }
}

/**
 * @brief Render a single entity with its drawable component.
 *
 * Applies transform properties and draws the sprite, considering
 * any associated shader.
 *
 * @param transforms Sparse array of Transform components.
 * @param drawables Sparse array of Drawable components.
 * @param shaders Sparse array of Shader components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param game_world The game world containing the render window.
 * @param i The index of the entity to render.
 */
void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    GameWorld &game_world, int i) {
    // Safety check for valid components
    if (!transforms.has(i) || !drawables.has(i)) {
        std::cerr << "[RenderOneEntity] Invalid entity at index " << i
                  << std::endl;
        return;
    }
    auto &transform = transforms[i];
    auto &drawable = drawables[i];

    if (!transform.has_value() || !drawable.has_value()) {
        std::cerr << "[RenderOneEntity] Missing value at index " << i
                  << std::endl;
        return;
    }

    std::optional<Com::Shader *> shaderCompOpt = std::nullopt;
    if (shaders.has(i) && shaders[i].has_value())
        shaderCompOpt = &shaders[i].value();

    // Calculate world position with hierarchical rotation
    sf::Vector2f world_position = ToSFML(
        CalculateWorldPositionWithHierarchy(transform.value(), transforms));

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
        ToSFML(CalculateCumulativeScale(transform.value(), transforms));
    drawable->sprite.setScale(world_scale);
    drawable->sprite.setRotation(transform->rotationDegrees);
    // Child rotation only: apply the entity's own rotation
    Engine::Graphics::Color engine_color = drawable->color;
    engine_color.a = static_cast<uint8_t>(drawable->opacity * 255);
    sf::Color color = ToSFML(engine_color);
    drawable->sprite.setColor(color);
    DrawSprite(game_world, drawable->sprite, &drawable.value(), shaderCompOpt);
}

/**
 * @brief System to render all drawable entities and particle emitters.
 *
 * Collects all drawable entities and active particle emitters,
 * sorts them by z-index, and renders them in order.
 *
 * @param reg Engine registry.
 * @param game_world The game world containing the render window.
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
    // Get RectangleDrawable components for rendering
    auto &rect_drawables = reg.GetComponents<Com::RectangleDrawable>();

    struct RenderItem {
        int index;
        int z_index;
        enum class Type {
            kSprite,
            kParticle,
            kRectangle
        } type;
    };

    std::vector<RenderItem> render_order;

    // Collect drawable entities (sprite-based)
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        RenderItem item;
        item.index = i;
        item.z_index = drawable.z_index;
        item.type = RenderItem::Type::kSprite;
        render_order.push_back(item);
    }

    // Collect RectangleDrawable entities
    for (auto &&[i, transform, rect_drawable] :
        make_indexed_zipper(transforms, rect_drawables)) {
        // Skip if shape pointer is null
        if (!rect_drawable.shape) {
            continue;
        }
        // Initialize rectangle shape if needed
        if (!rect_drawable.is_initialized) {
            rect_drawable.shape->setSize(
                sf::Vector2f(rect_drawable.width, rect_drawable.height));
            rect_drawable.shape->setOrigin(
                rect_drawable.width / 2.0f, rect_drawable.height / 2.0f);
            rect_drawable.shape->setFillColor(
                ToSFML(rect_drawable.fill_color));
            rect_drawable.shape->setOutlineColor(
                ToSFML(rect_drawable.outline_color));
            rect_drawable.shape->setOutlineThickness(
                rect_drawable.outline_thickness);
            rect_drawable.is_initialized = true;
        }
        RenderItem item;
        item.index = i;
        item.z_index = rect_drawable.z_index;
        item.type = RenderItem::Type::kRectangle;
        render_order.push_back(item);
    }

    // Collect particle emitters
    for (auto &&[i, transform, emitter] :
        make_indexed_zipper(transforms, emitters)) {
        if (emitter.active) {
            RenderItem item;
            item.index = i;
            item.z_index = emitter.z_index;
            item.type = RenderItem::Type::kParticle;
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
        if (item.type == RenderItem::Type::kParticle) {
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
        } else if (item.type == RenderItem::Type::kRectangle) {
            // Safety check for valid index
            if (!transforms.has(item.index) ||
                !rect_drawables.has(item.index)) {
                std::cerr
                    << "[DrawableSystem] Invalid rectangle entity at index "
                    << item.index << std::endl;
                continue;
            }
            auto &transform = transforms[item.index];
            auto &rect_drawable = rect_drawables[item.index];

            if (!transform.has_value() || !rect_drawable.has_value()) {
                std::cerr
                    << "[DrawableSystem] Missing value for rectangle at index "
                    << item.index << std::endl;
                continue;
            }

            // Skip if shape pointer is null
            if (!rect_drawable->shape) {
                continue;
            }

            // Calculate world position
            sf::Vector2f world_position =
                ToSFML(CalculateWorldPositionWithHierarchy(
                    transform.value(), transforms));
            rect_drawable->shape->setPosition(world_position);

            // Apply scale
            sf::Vector2f world_scale = ToSFML(
                CalculateCumulativeScale(transform.value(), transforms));
            rect_drawable->shape->setScale(world_scale);

            // Apply rotation
            rect_drawable->shape->setRotation(transform->rotationDegrees);

            // Apply opacity
            sf::Color fill = rect_drawable->shape->getFillColor();
            fill.a = static_cast<uint8_t>(rect_drawable->opacity * 255);
            rect_drawable->shape->setFillColor(fill);

            sf::Color outline = rect_drawable->shape->getOutlineColor();
            outline.a = static_cast<uint8_t>(rect_drawable->opacity * 255);
            rect_drawable->shape->setOutlineColor(outline);

            game_world.window_.draw(*rect_drawable->shape);
        } else {
            RenderOneEntity(transforms, drawables, shaders, animated_sprites,
                game_world, item.index);
        }
    }
}
}  // namespace Rtype::Client
