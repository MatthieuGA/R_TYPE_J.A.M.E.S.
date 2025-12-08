#include <SFML/Graphics.hpp>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
/**
 * @brief Detects mouse hover and click events on clickable entities.
 *
 * This system iterates over entities with HitBox, Clickable, Drawable, and
 * Transform components, determines if the mouse is hovering over or clicking
 * them, updates their state accordingly, and triggers the onClick callback
 * when a click is detected.
 *
 * @param reg Engine registry.
 * @param game_world Reference to the game world, including the window.
 * @param hit_boxes Sparse array of HitBox components.
 * @param clickables Sparse array of Clickable components.
 * @param drawables Sparse array of Drawable components.
 * @param transforms Sparse array of Transform components.
 */
void ButtonClickSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::HitBox> &hit_boxes,
    Eng::sparse_array<Com::Clickable> &clickables,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Transform> &transforms) {
    for (auto &&[i, hit_box, clickable, drawable, transform] :
        make_indexed_zipper(hit_boxes, clickables, drawables, transforms)) {
        sf::Vector2f mousePos = game_world.window_.mapPixelToCoords(
            sf::Mouse::getPosition(game_world.window_));

        const float width_computed =
            hit_box.width *
            (hit_box.scaleWithTransform ? transform.scale : 1.0f);
        const float height_computed =
            hit_box.height *
            (hit_box.scaleWithTransform ? transform.scale : 1.0f);

        const sf::Vector2f offsetOrigin = GetOffsetFromTransform(
            transform, sf::Vector2f(width_computed, height_computed));

        const float left = transform.x + offsetOrigin.x;
        const float top = transform.y + offsetOrigin.y;
        const float right = left + width_computed;
        const float bottom = top + height_computed;
        const bool isHovered = (mousePos.x >= left && mousePos.x <= right &&
                                mousePos.y >= top && mousePos.y <= bottom);

        clickable.isHovered = isHovered;
        if (isHovered && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            clickable.isClicked = true;
        } else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                   clickable.isClicked) {
            // Mouse button released after being clicked
            clickable.isClicked = false;
            if (clickable.onClick)
                clickable.onClick();
        } else {
            clickable.isClicked = false;
        }
        drawable.color = clickable.isClicked
                             ? clickable.clickColor
                             : (clickable.isHovered ? clickable.hoverColor
                                                    : clickable.idleColor);
    }
}
}  // namespace Rtype::Client
