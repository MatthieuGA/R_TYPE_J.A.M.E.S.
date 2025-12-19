#include "engine/CollidingTools.hpp"

#include "engine/OriginTool.hpp"

namespace Rtype::Client {

/**
 * @brief Check AABB collision using precomputed offsets and scaled hitboxes.
 *
 * @param trans_a Transform of entity A
 * @param hb_a HitBox of entity A
 * @param trans_b Transform of entity B
 * @param hb_b HitBox of entity B
 * @param off_a Offset for entity A (unscaled)
 * @param off_b Offset for entity B (unscaled)
 * @return true if the boxes overlap
 */
bool IsCollidingFromOffset(const Com::Transform &trans_a,
    const Com::HitBox &hb_a, const Com::Transform &trans_b,
    const Com::HitBox &hb_b, Engine::Graphics::Vector2f off_a,
    Engine::Graphics::Vector2f off_b) {
    const float scale_x_a =
        hb_a.scaleWithTransform ? std::abs(trans_a.scale.x) : 1.0f;
    const float scale_y_a =
        hb_a.scaleWithTransform ? std::abs(trans_a.scale.y) : 1.0f;
    const float scale_x_b =
        hb_b.scaleWithTransform ? std::abs(trans_b.scale.x) : 1.0f;
    const float scale_y_b =
        hb_b.scaleWithTransform ? std::abs(trans_b.scale.y) : 1.0f;

    Engine::Graphics::Vector2f scal_off_a =
        Engine::Graphics::Vector2f(off_a.x * scale_x_a, off_a.y * scale_y_a);
    Engine::Graphics::Vector2f scal_off_b =
        Engine::Graphics::Vector2f(off_b.x * scale_x_b, off_b.y * scale_y_b);
    Engine::Graphics::Vector2f scal_hb_a = Engine::Graphics::Vector2f(
        hb_a.width * scale_x_a, hb_a.height * scale_y_a);
    Engine::Graphics::Vector2f scal_hb_b = Engine::Graphics::Vector2f(
        hb_b.width * scale_x_b, hb_b.height * scale_y_b);

    if (trans_a.x + scal_off_a.x < trans_b.x + scal_hb_b.x + scal_off_b.x &&
        trans_a.x + scal_hb_a.x + scal_off_a.x > trans_b.x + scal_off_b.x &&
        trans_a.y + scal_off_a.y < trans_b.y + scal_hb_b.y + scal_off_b.y &&
        trans_a.y + scal_hb_a.y + scal_off_a.y > trans_b.y + scal_off_b.y) {
        return true;
    }
    return false;
}

/**
 * @brief Check AABB collision between two entities.
 *
 * This function computes the offsets based on the transforms and hitboxes,
 * then calls `IsCollidingFromOffset` to determine if a collision occurs.
 *
 * @param trans_a Transform of entity A
 * @param hb_a HitBox of entity A
 * @param trans_b Transform of entity B
 * @param hb_b HitBox of entity B
 * @return true if the boxes overlap
 */
bool IsColliding(const Com::Transform &trans_a, const Com::HitBox &hb_a,
    const Com::Transform &trans_b, const Com::HitBox &hb_b) {
    Engine::Graphics::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {hb_a.width, hb_a.height});
    Engine::Graphics::Vector2f off_b =
        GetOffsetFromTransform(trans_b, {hb_b.width, hb_b.height});

    return IsCollidingFromOffset(trans_a, hb_a, trans_b, hb_b, off_a, off_b);
}

}  // namespace Rtype::Client
