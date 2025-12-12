#include <algorithm>

#include "engine/CollidingTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/events/EngineEvent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

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
 * @param game_world Game world context (unused in current implementation)
 * @return true if the boxes overlap
 */
bool IsColliding(const Com::Transform &trans_a, const Com::HitBox &hb_a,
    const Com::Transform &trans_b, const Com::HitBox &hb_b,
    Rtype::Client::GameWorld &game_world) {
    Engine::Graphics::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {hb_a.width, hb_a.height});
    Engine::Graphics::Vector2f off_b =
        GetOffsetFromTransform(trans_b, {hb_b.width, hb_b.height});

    return IsCollidingFromOffset(trans_a, hb_a, trans_b, hb_b, off_a, off_b);
}

/**
 * @brief Compute and resolve penetration between two colliding entities.
 *
 * This function will check solidity and lock flags and move one or both
 * entities along the smallest penetration axis so they no longer overlap.
 *
 * @param solids Sparse array of Solid components to query solidity/lock
 * @param i Index of entity A
 * @param j Index of entity B
 * @param trans_a Transform of entity A (may be modified)
 * @param hb_a HitBox of entity A
 * @param trans_b Transform of entity B (may be modified)
 * @param hb_b HitBox of entity B
 */
void ComputeCollision(Eng::sparse_array<Com::Solid> const &solids, int i,
    int j, Com::Transform &trans_a, const Com::HitBox &hb_a,
    Com::Transform &trans_b, const Com::HitBox &hb_b) {
    // If none are solid, nothing to resolve here
    bool aSolid = solids.has(i) ? solids[i]->isSolid : false;
    bool bSolid = solids.has(j) ? solids[j]->isSolid : false;
    if (!aSolid && !bSolid)
        return;

    // Check stuck/locked flags: if an entity is stuck, it must not be moved
    bool aStuck = solids.has(i) ? solids[i]->isLocked : false;
    bool bStuck = solids.has(j) ? solids[j]->isLocked : false;
    // If both are stuck, nothing to resolve
    if (aStuck && bStuck)
        return;

    float width_computed_a =
        hb_a.width * (hb_a.scaleWithTransform ? trans_a.scale.x : 1.0f);
    float height_computed_a =
        hb_a.height * (hb_a.scaleWithTransform ? trans_a.scale.y : 1.0f);
    float width_computed_b =
        hb_b.width * (hb_b.scaleWithTransform ? trans_b.scale.x : 1.0f);
    float height_computed_b =
        hb_b.height * (hb_b.scaleWithTransform ? trans_b.scale.y : 1.0f);

    // Compute offsets and scaled hitboxes (same as is_colliding)
    Engine::Graphics::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {width_computed_a, height_computed_a});
    Engine::Graphics::Vector2f off_b =
        GetOffsetFromTransform(trans_b, {width_computed_b, height_computed_b});

    Engine::Graphics::Vector2f scal_off_a = off_a * trans_a.scale;
    Engine::Graphics::Vector2f scal_off_b = off_b * trans_b.scale;
    Engine::Graphics::Vector2f hb_scal_a = Engine::Graphics::Vector2f(
        width_computed_a * trans_a.scale.x, height_computed_a * trans_a.scale.y);
    Engine::Graphics::Vector2f hb_scal_b = Engine::Graphics::Vector2f(
        width_computed_b * trans_b.scale.x, height_computed_b * trans_b.scale.y);

    float a_min_x = trans_a.x + scal_off_a.x;
    float a_max_x = a_min_x + hb_scal_a.x;
    float a_min_y = trans_a.y + scal_off_a.y;
    float a_max_y = a_min_y + hb_scal_a.y;

    float b_min_x = trans_b.x + scal_off_b.x;
    float b_max_x = b_min_x + hb_scal_b.x;
    float b_min_y = trans_b.y + scal_off_b.y;
    float b_max_y = b_min_y + hb_scal_b.y;

    float overlap_x = std::min(a_max_x, b_max_x) - std::max(a_min_x, b_min_x);
    float overlap_y = std::min(a_max_y, b_max_y) - std::max(a_min_y, b_min_y);

    if (overlap_x <= 0 || overlap_y <= 0)
        return;

    // Separate along smallest penetration axis
    if (overlap_x < overlap_y) {
        float a_center = (a_min_x + a_max_x) / 2.0f;
        float b_center = (b_min_x + b_max_x) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool a_can_move = aSolid && !aStuck;
        bool b_can_move = bSolid && !bStuck;
        if (a_can_move && b_can_move) {
            trans_a.x += direction * (overlap_x / 2.0f);
            trans_b.x -= direction * (overlap_x / 2.0f);
        } else if (a_can_move && !b_can_move) {
            trans_a.x += direction * overlap_x;
        } else if (!a_can_move && b_can_move) {
            trans_b.x -= direction * overlap_x;
        }
    } else {
        float a_center = (a_min_y + a_max_y) / 2.0f;
        float b_center = (b_min_y + b_max_y) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool a_can_move = aSolid && !aStuck;
        bool b_can_move = bSolid && !bStuck;
        if (a_can_move && b_can_move) {
            trans_a.y += direction * (overlap_y / 2.0f);
            trans_b.y -= direction * (overlap_y / 2.0f);
        } else if (a_can_move && !b_can_move) {
            trans_a.y += direction * overlap_y;
        } else if (!a_can_move && b_can_move) {
            trans_b.y -= direction * overlap_y;
        }
    }
}

/**
 * @brief System that detects collisions for all entities with Transform and
 * HitBox components and publishes collision events.
 *
 * For each pair of entities this system tests AABB overlap, publishes a
 * CollisionEvent on the `game_world.event_bus_` when a collision is found and
 * calls `ComputeCollision` to resolve penetration for solid entities.
 *
 * @param reg Engine registry (unused in current implementation)
 * @param game_world Game world context (window size, event bus, etc.)
 * @param transforms Sparse array of Transform components
 * @param hitBoxes Sparse array of HitBox components
 * @param solids Sparse array of Solid components
 */
void CollisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids) {
    // Placeholder for collision detection logic
    for (auto &&[i, trans_a, hb_a] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, trans_b, hb_b] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j)
                continue;
            if (IsColliding(trans_a, hb_a, trans_b, hb_b)) {
                game_world.event_bus_.Publish(
                    CollisionEvent{i, j, game_world});
                ComputeCollision(solids, i, j, trans_a, hb_a, trans_b, hb_b);
            }
        }
    }
}
}  // namespace Rtype::Client
