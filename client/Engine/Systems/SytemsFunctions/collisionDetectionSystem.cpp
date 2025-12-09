#include <algorithm>

#include "Engine/Events/EngineEvent.hpp"
#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

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
    const Com::HitBox &hb_b, sf::Vector2f off_a, sf::Vector2f off_b) {
    sf::Vector2f scal_off_a = off_a * trans_a.scale;
    sf::Vector2f scal_off_b = off_b * trans_b.scale;
    sf::Vector2f scal_hb_a =
        sf::Vector2f(hb_a.width * trans_a.scale, hb_a.height * trans_a.scale);
    sf::Vector2f scal_hb_b =
        sf::Vector2f(hb_b.width * trans_b.scale, hb_b.height * trans_b.scale);

    if (trans_a.x + scal_off_a.x < trans_b.x + scal_hb_b.x + scal_off_b.x &&
        trans_a.x + scal_hb_a.x + scal_off_a.x > trans_b.x + scal_off_b.x &&
        trans_a.y + scal_off_a.y < trans_b.y + scal_hb_b.y + scal_off_b.y &&
        trans_a.y + scal_hb_a.y + scal_off_a.y > trans_b.y + scal_off_b.y) {
        return true;
    }
    return false;
}

bool IsColliding(const Com::Transform &trans_a, const Com::HitBox &hb_a,
    const Com::Transform &trans_b, const Com::HitBox &hb_b,
    Rtype::Client::GameWorld &game_world) {
    sf::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {hb_a.width, hb_a.height});
    sf::Vector2f off_b =
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
        hb_a.width * (hb_a.scaleWithTransform ? trans_a.scale : 1.0f);
    float height_computed_a =
        hb_a.height * (hb_a.scaleWithTransform ? trans_a.scale : 1.0f);
    float width_computed_b =
        hb_b.width * (hb_b.scaleWithTransform ? trans_b.scale : 1.0f);
    float height_computed_b =
        hb_b.height * (hb_b.scaleWithTransform ? trans_b.scale : 1.0f);

    // Compute offsets and scaled hitboxes (same as is_colliding)
    sf::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {width_computed_a, height_computed_a});
    sf::Vector2f off_b =
        GetOffsetFromTransform(trans_b, {width_computed_b, height_computed_b});

    sf::Vector2f scal_off_a = off_a * trans_a.scale;
    sf::Vector2f scal_off_b = off_b * trans_b.scale;
    sf::Vector2f scaledA = sf::Vector2f(
        width_computed_a * trans_a.scale, height_computed_a * trans_a.scale);
    sf::Vector2f scaledB = sf::Vector2f(
        width_computed_b * trans_b.scale, height_computed_b * trans_b.scale);

    float a_min_x = trans_a.x + scal_off_a.x;
    float a_max_x = a_min_x + scaledA.x;
    float a_min_y = trans_a.y + scal_off_a.y;
    float a_max_y = a_min_y + scaledA.y;

    float b_min_x = trans_b.x + scal_off_b.x;
    float b_max_x = b_min_x + scaledB.x;
    float b_min_y = trans_b.y + scal_off_b.y;
    float b_max_y = b_min_y + scaledB.y;

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
            if (IsColliding(trans_a, hb_a, trans_b, hb_b, game_world)) {
                game_world.event_bus_.Publish(
                    CollisionEvent{i, j, game_world});
                ComputeCollision(solids, i, j, trans_a, hb_a, trans_b, hb_b);
            }
        }
    }
}
}  // namespace Rtype::Client
