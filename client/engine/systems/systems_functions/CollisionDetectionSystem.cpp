#include <algorithm>

#include "engine/CollidingTools.hpp"
#include "engine/OriginTool.hpp"
#include "engine/events/EngineEvent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Compute and resolve penetration between two colliding entities.
 *
 * This function will check solidity and lock flags and move one or both
 * entities along the smallest penetration axis so they no longer overlap.
 *
 * The collision resolution logic matches the server's ObstacleCollisionSystem
 * to ensure consistent behavior between client and server.
 *
 * Collision resolution rules:
 * - If neither entity is solid, no resolution needed
 * - If both entities are locked, no resolution possible
 * - If one entity is solid+locked, the other entity is pushed away
 * - If both entities are solid and unlocked, both are pushed equally
 * - An entity without a Solid component can still be pushed by a solid entity
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
    // Get solid flags (default to not solid if no component)
    bool aSolid = solids.has(i) ? solids[i]->isSolid : false;
    bool bSolid = solids.has(j) ? solids[j]->isSolid : false;
    // If neither entity is solid, nothing to resolve
    if (!aSolid && !bSolid)
        return;

    // Check locked flags: locked entities cannot be moved
    bool aLocked = solids.has(i) ? solids[i]->isLocked : false;
    bool bLocked = solids.has(j) ? solids[j]->isLocked : false;
    // If both are locked, no resolution possible
    if (aLocked && bLocked)
        return;

    // Calculate scaled hitbox dimensions (using abs for scale like server)
    float width_a =
        hb_a.width * (hb_a.scaleWithTransform
                          ? std::abs(trans_a.scale.x)
                          : 1.0f);
    float height_a =
        hb_a.height * (hb_a.scaleWithTransform
                           ? std::abs(trans_a.scale.y)
                           : 1.0f);
    float width_b =
        hb_b.width * (hb_b.scaleWithTransform
                          ? std::abs(trans_b.scale.x)
                          : 1.0f);
    float height_b =
        hb_b.height * (hb_b.scaleWithTransform
                           ? std::abs(trans_b.scale.y)
                           : 1.0f);

    // Get offsets based on origin (pass already-scaled dimensions)
    Engine::Graphics::Vector2f off_a =
        GetOffsetFromTransform(trans_a, {width_a, height_a});
    Engine::Graphics::Vector2f off_b =
        GetOffsetFromTransform(trans_b, {width_b, height_b});

    // Scale offsets (using abs for scale like server)
    Engine::Graphics::Vector2f scal_off_a = Engine::Graphics::Vector2f(
        off_a.x * std::abs(trans_a.scale.x),
        off_a.y * std::abs(trans_a.scale.y));
    Engine::Graphics::Vector2f scal_off_b = Engine::Graphics::Vector2f(
        off_b.x * std::abs(trans_b.scale.x),
        off_b.y * std::abs(trans_b.scale.y));

    // Calculate AABB bounds (width_a/height_a already scaled, don't scale again)
    float a_min_x = trans_a.x + scal_off_a.x;
    float a_max_x = a_min_x + width_a;
    float a_min_y = trans_a.y + scal_off_a.y;
    float a_max_y = a_min_y + height_a;

    float b_min_x = trans_b.x + scal_off_b.x;
    float b_max_x = b_min_x + width_b;
    float b_min_y = trans_b.y + scal_off_b.y;
    float b_max_y = b_min_y + height_b;

    float overlap_x = std::min(a_max_x, b_max_x) - std::max(a_min_x, b_min_x);
    float overlap_y = std::min(a_max_y, b_max_y) - std::max(a_min_y, b_min_y);

    if (overlap_x <= 0 || overlap_y <= 0)
        return;

    // Determine which entities can be moved
    // An entity can move if: it's not locked AND (it's solid OR the other is
    // solid+locked) This allows non-solid entities to be pushed by solid+locked
    // objects
    bool a_can_move = !aLocked && (aSolid || (bSolid && bLocked));
    bool b_can_move = !bLocked && (bSolid || (aSolid && aLocked));

    // Separate along smallest penetration axis
    if (overlap_x < overlap_y) {
        float a_center = (a_min_x + a_max_x) / 2.0f;
        float b_center = (b_min_x + b_max_x) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
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
 * IMPORTANT: Collision resolution is ONLY applied when at least one entity
 * is controllable (local player). This ensures the server remains authoritative
 * for non-local entities while still providing client-side prediction for the
 * local player.
 *
 * @param reg Engine registry (unused in current implementation)
 * @param game_world Game world context (window size, event bus, etc.)
 * @param transforms Sparse array of Transform components
 * @param hitBoxes Sparse array of HitBox components
 * @param solids Sparse array of Solid components
 * @param controllables Sparse array of Controllable components
 */
void CollisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids,
    Eng::sparse_array<Com::Controllable> const &controllables) {
    // Detect and resolve collisions
    for (auto &&[i, trans_a, hb_a] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, trans_b, hb_b] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j)
                continue;
            if (IsColliding(trans_a, hb_a, trans_b, hb_b)) {
                // Always publish collision events for gameplay logic
                game_world.event_bus_.Publish(
                    CollisionEvent{i, j, game_world});

                // Only resolve collision if at least one entity is controllable
                // (local player). Non-local entities use server positions.
                bool i_controllable = controllables.has(i) &&
                    controllables[i].has_value() &&
                    controllables[i]->isControllable;
                bool j_controllable = controllables.has(j) &&
                    controllables[j].has_value() &&
                    controllables[j]->isControllable;

                if (i_controllable || j_controllable) {
                    ComputeCollision(
                        solids, i, j, trans_a, hb_a, trans_b, hb_b);
                }
            }
        }
    }
}
}  // namespace Rtype::Client
