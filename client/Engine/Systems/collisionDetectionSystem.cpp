#include "Engine/initRegistrySystems.hpp"
#include "Engine/Events/EngineEvent.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
bool IsCollidingFromOffset(const Com::Transform &transform_a, const Com::HitBox &hit_box_a,
    const Com::Transform &transform_b, const Com::HitBox &hit_box_b,
    sf::Vector2f offset_a, sf::Vector2f offset_b) {
    sf::Vector2f offset_a_scaled = offset_a * transform_a.scale;
    sf::Vector2f offset_b_scaled = offset_b * transform_b.scale;
    sf::Vector2f scaled_hit_box_a = sf::Vector2f(hit_box_a.width * transform_a.scale,
        hit_box_a.height * transform_a.scale);
    sf::Vector2f scaled_hit_box_b = sf::Vector2f(hit_box_b.width * transform_b.scale,
        hit_box_b.height * transform_b.scale);

    if (transform_a.x + offset_a_scaled.x < transform_b.x + scaled_hit_box_b.x + offset_b_scaled.x &&
        transform_a.x + scaled_hit_box_a.x + offset_a_scaled.x > transform_b.x + offset_b_scaled.x &&
        transform_a.y + offset_a_scaled.y < transform_b.y + scaled_hit_box_b.y + offset_b_scaled.y &&
        transform_a.y + scaled_hit_box_a.y + offset_a_scaled.y > transform_b.y + offset_b_scaled.y) {
        return true;
    }
    return false;
}

bool IsColliding(const Com::Transform &transform_a, const Com::HitBox &hit_box_a,
    const Com::Transform &transform_b, const Com::HitBox &hit_box_b, Rtype::Client::GameWorld &game_world) {
    sf::Vector2f offset_a = GetOffsetFromTransform(transform_a, {hit_box_a.width, hit_box_a.height});
    sf::Vector2f offset_b = GetOffsetFromTransform(transform_b, {hit_box_b.width, hit_box_b.height});

    return IsCollidingFromOffset(transform_a, hit_box_a, transform_b, hit_box_b, offset_a, offset_b);
}

void ComputeCollision(Eng::sparse_array<Com::Solid> const &solids, int i, int j,
    Com::Transform &transform_a, const Com::HitBox &hit_box_a,
    Com::Transform &transform_b, const Com::HitBox &hit_box_b) {
    // If none are solid, nothing to resolve here
    bool aSolid = solids.has(i) ? solids[i]->isSolid : false;
    bool bSolid = solids.has(j) ? solids[j]->isSolid : false;
    if (!aSolid && !bSolid) return;

    // Check stuck/locked flags: if an entity is stuck, it must not be moved
    bool aStuck = solids.has(i) ? solids[i]->isLocked : false;
    bool bStuck = solids.has(j) ? solids[j]->isLocked : false;
    // If both are stuck, nothing to resolve
    if (aStuck && bStuck) return;

    // Compute offsets and scaled hitboxes (same as is_colliding)
    sf::Vector2f offset_a = GetOffsetFromTransform(transform_a, {hit_box_a.width, hit_box_a.height});
    sf::Vector2f offset_b = GetOffsetFromTransform(transform_b, {hit_box_b.width, hit_box_b.height});

    sf::Vector2f offset_a_scaled = offset_a * transform_a.scale;
    sf::Vector2f offset_b_scaled = offset_b * transform_b.scale;
    sf::Vector2f scaledA = sf::Vector2f(hit_box_a.width * transform_a.scale,
        hit_box_a.height * transform_a.scale);
    sf::Vector2f scaledB = sf::Vector2f(hit_box_b.width * transform_b.scale,
        hit_box_b.height * transform_b.scale);

    float a_min_x = transform_a.x + offset_a_scaled.x;
    float a_max_x = a_min_x + scaledA.x;
    float a_min_y = transform_a.y + offset_a_scaled.y;
    float a_max_y = a_min_y + scaledA.y;

    float b_min_x = transform_b.x + offset_b_scaled.x;
    float b_max_x = b_min_x + scaledB.x;
    float b_min_y = transform_b.y + offset_b_scaled.y;
    float b_max_y = b_min_y + scaledB.y;

    float overlap_x = std::min(a_max_x, b_max_x) - std::max(a_min_x, b_min_x);
    float overlap_y = std::min(a_max_y, b_max_y) - std::max(a_min_y, b_min_y);

    if (overlap_x <= 0 || overlap_y <= 0) return;

    // Separate along smallest penetration axis
    if (overlap_x < overlap_y) {
        float a_center = (a_min_x + a_max_x) / 2.0f;
        float b_center = (b_min_x + b_max_x) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool a_can_move = aSolid && !aStuck;
        bool b_can_move = bSolid && !bStuck;
        if (a_can_move && b_can_move) {
            transform_a.x += direction * (overlap_x / 2.0f);
            transform_b.x -= direction * (overlap_x / 2.0f);
        } else if (a_can_move && !b_can_move) {
            transform_a.x += direction * overlap_x;
        } else if (!a_can_move && b_can_move) {
            transform_b.x -= direction * overlap_x;
        }
    } else {
        float a_center = (a_min_y + a_max_y) / 2.0f;
        float b_center = (b_min_y + b_max_y) / 2.0f;
        float direction = (a_center < b_center) ? -1.0f : 1.0f;
        bool a_can_move = aSolid && !aStuck;
        bool b_can_move = bSolid && !bStuck;
        if (a_can_move && b_can_move) {
            transform_a.y += direction * (overlap_y / 2.0f);
            transform_b.y -= direction * (overlap_y / 2.0f);
        } else if (a_can_move && !b_can_move) {
            transform_a.y += direction * overlap_y;
        } else if (!a_can_move && b_can_move) {
            transform_b.y -= direction * overlap_y;
        }
    }
}

void CollisionDetectionSystem(Eng::registry &reg, Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids) {
    // Placeholder for collision detection logic
    for (auto &&[i, transform_a, hit_box_a] :
        make_indexed_zipper(transforms, hitBoxes)) {
        for (auto &&[j, transform_b, hit_box_b] :
            make_indexed_zipper(transforms, hitBoxes)) {
            if (i >= j) continue;
            if (IsColliding(transform_a, hit_box_a, transform_b, hit_box_b, game_world)) {
                game_world.event_bus_.Publish(CollisionEvent{i, j, game_world});
                ComputeCollision(solids, i, j, transform_a, hit_box_a, transform_b, hit_box_b);
            }
        }
    }
}
}  // namespace Rtype::Client
