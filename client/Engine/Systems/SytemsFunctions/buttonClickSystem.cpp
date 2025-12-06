#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
/**
 * @brief Poll keyboard state and populate the Inputs components.
 *
 * This system resets each `Inputs` entry and then maps a fixed set of keys
 * to the input axes and shoot flag.
 *
 * @param reg Engine registry (unused)
 * @param inputs Sparse array of Inputs components to update
 */
void buttonClickSystem(Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::HitBox> &hit_boxes,
Eng::sparse_array<Com::Clickable> &clickables,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::Transform> &transforms) {
    for (auto &&[i, hit_box, clickable, drawable, transform] :
    make_indexed_zipper(hit_boxes, clickables, drawables, transforms)) {
        sf::Vector2f mousePos = game_world.window_.mapPixelToCoords(
            sf::Mouse::getPosition(game_world.window_));

        sf::Vector2f offsetOrigin = GetOffsetFromTransform(transform,
            sf::Vector2f(hit_box.width, hit_box.height));

        float left = transform.x + offsetOrigin.x;
        float top = transform.y + offsetOrigin.y;
        float right = left + hit_box.width;
        float bottom = top + hit_box.height;
        bool isHovered = (mousePos.x >= left && mousePos.x <= right &&
                          mousePos.y >= top && mousePos.y <= bottom);
        clickable.isHovered = isHovered;
        if (isHovered && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            clickable.isClicked = true;
        } else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && clickable.isClicked) {
            // Mouse button released after being clicked
            clickable.isClicked = false;
            if (clickable.onClick)
                clickable.onClick();
        } else {
            clickable.isClicked = false;
        }
        drawable.color = clickable.isClicked ? clickable.clickColor :
            (clickable.isHovered ? clickable.hoverColor : clickable.idleColor);
    }
}
}  // namespace Rtype::Client
