#include <SFML/Graphics.hpp>

#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Set the opacity of child drawable based on player's charge time.
 *
 * @param drawables Sparse array of Drawable components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param player_tag The PlayerTag component of the parent entity.
 * @param transforms Sparse array of Transform components.
 * @param child_id The ID of the child entity whose drawable to update.
 */
void SetOpacityChildren(Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Com::PlayerTag &player_tag, Eng::sparse_array<Com::Transform> &transforms,
    size_t child_id) {
    auto &drawable = drawables[child_id];
    auto &animated_sprite = animated_sprites[child_id];

    if (drawable->opacity == 0.0f)
        animated_sprite->currentFrame = 0;
    drawable->opacity =
        (player_tag.charge_time > player_tag.charge_time_min) ? 1.0f : 0.0f;
}

/**
 * @brief System to show charging asset for player based on charge time.
 *
 * This system updates the opacity of the charging asset child entity
 * based on the player's charge time.
 *
 * @param reg The registry containing components.
 * @param player_tags Sparse array of PlayerTag components.
 * @param drawables Sparse array of Drawable components.
 * @param animated_sprites Sparse array of AnimatedSprite components.
 * @param transforms Sparse array of Transform components.
 */
void ChargingShowAssetPlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::Transform> &transforms) {
    for (auto &&[i, player_tag, transform] :
        make_indexed_zipper(player_tags, transforms)) {
        // Update opacity for all children based on charge_time

        if (transform.children.empty())
            continue;
        size_t child_id = transform.children[0];
        if (child_id < drawables.size() && drawables[child_id].has_value()) {
            SetOpacityChildren(
                drawables, animated_sprites, player_tag, transforms, child_id);
        }
    }
}
}  // namespace Rtype::Client
