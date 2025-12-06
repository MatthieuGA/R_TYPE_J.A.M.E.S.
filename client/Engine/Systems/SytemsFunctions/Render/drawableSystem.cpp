#include <iostream>
#include <vector>
#include <algorithm>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

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
Com::Drawable *drawable, std::optional<Com::Shader*> shaderCompOpt) {
    if (shaderCompOpt.has_value() && (*shaderCompOpt)->isLoaded) {
        ((*shaderCompOpt)->shader)->setUniform("time",
            game_world.total_time_clock_.getElapsedTime().asSeconds());
        game_world.window_.draw(sprite,
            sf::RenderStates((*shaderCompOpt)->shader.get()));
    } else {
        game_world.window_.draw(sprite);
    }
}

/**
 * @brief Calculates the world position accounting for hierarchical rotation.
 *
 * When a child has a parent, it orbits around the parent's origin.
 * The position is first rotated by the parent's rotation, then offset.
 *
 * @param transform The transform component of the entity.
 * @param transforms The sparse array of all transforms (to access parent).
 * @return The world position with rotational hierarchy applied.
 */
sf::Vector2f CalculateWorldPositionWithHierarchy(
    const Com::Transform &transform,
    const Eng::sparse_array<Com::Transform> &transforms) {
    if (!transform.parent_entity.has_value()) {
        return sf::Vector2f(transform.x, transform.y);
    }

    // Get parent entity ID
    std::size_t parent_id = transform.parent_entity.value();

    // Check if parent exists and has a Transform component
    if (!transforms.has(parent_id)) {
        return sf::Vector2f(transform.x, transform.y);
    }

    const Com::Transform &parent_transform = transforms[parent_id].value();

    // Recursively get parent's world position and rotation
    sf::Vector2f parent_pos =
        CalculateWorldPositionWithHierarchy(parent_transform, transforms);
    float parent_rotation_rad =
        parent_transform.rotationDegrees * 3.14159265f / 180.0f;

    // Add parent's parent rotations
    if (parent_transform.parent_entity.has_value()) {
        std::size_t grandparent_id = parent_transform.parent_entity.value();
        if (transforms.has(grandparent_id)) {
            const Com::Transform &grandparent =
                transforms[grandparent_id].value();
            parent_rotation_rad +=
                grandparent.GetWorldRotation() * 3.14159265f / 180.0f;
        }
    }

    // Calculate local position relative to parent's origin
    float local_x = transform.x;
    float local_y = transform.y;

    // Rotate local position by parent's rotation
    float rotated_x = local_x * std::cos(parent_rotation_rad) -
        local_y * std::sin(parent_rotation_rad);
    float rotated_y = local_x * std::sin(parent_rotation_rad) +
        local_y * std::cos(parent_rotation_rad);

    // Apply parent's position
    return sf::Vector2f(parent_pos.x + rotated_x, parent_pos.y + rotated_y);
}

float CalculateCumulativeScale(
    const Com::Transform &transform,
    const Eng::sparse_array<Com::Transform> &transforms) {
    float cumulative_scale = transform.scale;

    if (transform.parent_entity.has_value()) {
        std::size_t parent_id = transform.parent_entity.value();
        if (transforms.has(parent_id)) {
            const Com::Transform &parent_transform =
                transforms[parent_id].value();
            cumulative_scale *=
                CalculateCumulativeScale(parent_transform, transforms);
        }
    }
    return cumulative_scale;
}

void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::Shader> &shaders,
GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];


    std::optional<Com::Shader*> shaderCompOpt = std::nullopt;
    if (shaders.has(i)) shaderCompOpt = &shaders[i].value();

    // Calculate world position with hierarchical rotation
    sf::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);
    drawable->sprite.setPosition(world_position);
    float world_scale = CalculateCumulativeScale(transform.value(), transforms);
    drawable->sprite.setScale(sf::Vector2f(world_scale, world_scale));
    // Child rotation only: apply the entity's own rotation
    drawable->sprite.setRotation(transform->rotationDegrees);
    drawable->sprite.setColor(sf::Color(255, 255, 255,
        drawable->opacity * 255));
    DrawSprite(game_world, drawable->sprite, &drawable.value(), shaderCompOpt);
}

void DrawableSystem(
Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, transform, drawable] :
    make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        draw_order.push_back(i);
    }

    // Sort by z_index
    std::sort(draw_order.begin(), draw_order.end(),
    [&drawables](int a, int b) {
        return drawables[a]->z_index < drawables[b]->z_index;
    });

    for (auto i : draw_order) {
        RenderOneEntity(transforms, drawables, shaders, game_world, i);
    }
}
}  // namespace Rtype::Client
