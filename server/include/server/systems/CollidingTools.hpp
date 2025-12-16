#pragma once

#include "include/sparse_array.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"

namespace server {

bool IsCollidingFromOffset(const Component::Transform &trans_a,
    const Component::HitBox &hb_a, const Component::Transform &trans_b,
    const Component::HitBox &hb_b, vector2f off_a, vector2f off_b);

bool IsColliding(const Component::Transform &trans_a,
    const Component::HitBox &hb_a, const Component::Transform &trans_b,
    const Component::HitBox &hb_b);

}  // namespace server
