#include "engine/hierarchyTools.hpp"

#include <cmath>
#include <numbers>

namespace Rtype::Client {

/**
 * @brief Calculates the cumulative scale of a transform including parent
 * scales.
 *
 * This function recursively multiplies the scale of the given transform
 * with the scales of its parent transforms up the hierarchy.
 *
 * @param transform The transform component of the entity.
 * @param transforms The sparse array of all transforms (to access parent).
 * @return The cumulative scale factor.
 */
float CalculateCumulativeScale(const Com::Transform &transform,
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
        parent_transform.rotationDegrees * std::numbers::pi / 180.0f;

    // Add parent's parent rotations
    if (parent_transform.parent_entity.has_value()) {
        std::size_t grandparent_id = parent_transform.parent_entity.value();
        if (transforms.has(grandparent_id)) {
            const Com::Transform &grandparent =
                transforms[grandparent_id].value();
            parent_rotation_rad +=
                grandparent.GetWorldRotation() * std::numbers::pi / 180.0f;
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

}  // namespace Rtype::Client
