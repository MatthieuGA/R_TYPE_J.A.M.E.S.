#pragma once

#include "engine/GameWorld.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/sparse_array.hpp"

namespace Rtype::Client {
namespace Com = Component;
namespace Eng = Engine;

bool IsColliding(const Com::Transform &trans_a, const Com::HitBox &hb_a,
    const Com::Transform &trans_b, const Com::HitBox &hb_b);

}  // namespace Rtype::Client
