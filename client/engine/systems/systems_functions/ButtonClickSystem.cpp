#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

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
 * @param game_world Reference to the game world, including mouse state.
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
    (void)reg;

    Engine::Graphics::Vector2f mousePos = game_world.mouse_position_;

    for (auto &&[i, hit_box, clickable, drawable, transform] :
        make_indexed_zipper(hit_boxes, clickables, drawables, transforms)) {
        const float width_computed =
            hit_box.width *
            (hit_box.scaleWithTransform ? transform.scale.x : 1.0f);
        const float height_computed =
            hit_box.height *
            (hit_box.scaleWithTransform ? transform.scale.y : 1.0f);

        const Engine::Graphics::Vector2f offsetOrigin =
            GetOffsetFromTransform(transform,
                Engine::Graphics::Vector2f(width_computed, height_computed));

        const float left = transform.x + offsetOrigin.x;
        const float top = transform.y + offsetOrigin.y;
        const float right = left + width_computed;
        const float bottom = top + height_computed;
        const bool isHovered = (mousePos.x >= left && mousePos.x <= right &&
                                mousePos.y >= top && mousePos.y <= bottom);

        clickable.isHovered = isHovered;
        if (isHovered && game_world.mouse_button_pressed_) {
            clickable.isClicked = true;
        } else if (!game_world.mouse_button_pressed_ && clickable.isClicked) {
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
