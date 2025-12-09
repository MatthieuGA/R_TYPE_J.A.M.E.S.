#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "engine/HierarchyTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

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
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    std::vector<int> draw_order;

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        draw_order.push_back(i);
    }

    // Sort by z_index
    std::sort(
        draw_order.begin(), draw_order.end(), [&drawables](int a, int b) {
            return drawables[a]->z_index < drawables[b]->z_index;
        });

    for (auto i : draw_order) {
        RenderOneEntity(
            transforms, drawables, shaders, animated_sprites, game_world, i);
    }
}
}  // namespace Rtype::Client
