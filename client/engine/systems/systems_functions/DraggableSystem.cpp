#include <algorithm>

#include <SFML/Graphics.hpp>

#include "engine/OriginTool.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "input/MouseButton.hpp"

namespace Rtype::Client {
/**
 * @brief Handles drag interactions for entities with Draggable components.
 *
 * This system manages mouse drag operations on entities, allowing them to
 * be moved by clicking and dragging. It applies position constraints and
 * triggers callbacks during drag events.
 *
 * @param reg Engine registry.
 * @param game_world Reference to the game world, including the window.
 * @param hit_boxes Sparse array of HitBox components.
 * @param draggables Sparse array of Draggable components.
 * @param transforms Sparse array of Transform components.
 */
void DraggableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::HitBox> &hit_boxes,
    Eng::sparse_array<Com::Draggable> &draggables,
    Eng::sparse_array<Com::Transform> &transforms) {
    auto &native_window = game_world.GetNativeWindow();
    if (!native_window.hasFocus())
        return;

    // TODO(MatthieuGA): mapPixelToCoords is SFML-specific,
    // will have to abstract
    sf::Vector2f sfMousePos = native_window.mapPixelToCoords(
        sf::Mouse::getPosition(native_window));
    Engine::Graphics::Vector2f mousePos(sfMousePos.x, sfMousePos.y);

    bool mousePressed = game_world.input_manager_->IsMouseButtonPressed(
        Engine::Input::MouseButton::Left);

    for (auto &&[i, hit_box, draggable, transform] :
        make_indexed_zipper(hit_boxes, draggables, transforms)) {
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

        if (!draggable.is_dragging) {
            // Check if drag should start
            if (isHovered && mousePressed) {
                draggable.is_dragging = true;
                draggable.drag_offset = Engine::Graphics::Vector2f(
                    mousePos.x - transform.x, mousePos.y - transform.y);

                // Call on_drag_start callback
                if (draggable.on_drag_start) {
                    draggable.on_drag_start(transform.x, transform.y);
                }
            }
        } else {
            // Currently dragging
            if (mousePressed) {
                // Calculate new position
                float new_x = mousePos.x - draggable.drag_offset.x;
                float new_y = mousePos.y - draggable.drag_offset.y;

                // Apply constraints
                if (draggable.constrain_vertical) {
                    new_y = transform.y;  // Lock Y position
                }
                if (draggable.constrain_horizontal) {
                    new_x = transform.x;  // Lock X position
                }

                // Clamp to min/max bounds
                new_x = std::max(
                    draggable.min_x, std::min(new_x, draggable.max_x));
                new_y = std::max(
                    draggable.min_y, std::min(new_y, draggable.max_y));

                // Update transform position
                transform.x = new_x;
                transform.y = new_y;

                // Call on_drag callback
                if (draggable.on_drag) {
                    draggable.on_drag(new_x, new_y);
                }
            } else {
                // Mouse released, end drag
                draggable.is_dragging = false;

                // Call on_drag_end callback
                if (draggable.on_drag_end) {
                    draggable.on_drag_end(transform.x, transform.y);
                }
            }
        }
    }
}
}  // namespace Rtype::Client
