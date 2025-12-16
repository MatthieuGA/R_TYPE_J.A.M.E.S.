#pragma once

#include "include/sparse_array.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"

namespace server {

bool IsColliding(const Component::Transform &trans_a,
    const Component::HitBox &hb_a, const Component::Transform &trans_b,
    const Component::HitBox &hb_b);

}  // namespace server
