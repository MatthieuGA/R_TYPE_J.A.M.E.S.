#pragma once

#include "engine/GameWorld.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/sparse_array.hpp"

namespace Rtype::Client {
namespace Com = Component;
namespace Eng = Engine;

bool IsColliding(const Com::Transform &trans_a, const Com::HitBox &hb_a,
    const Com::Transform &trans_b, const Com::HitBox &hb_b);

bool IsCollidingFromOffset(const Com::Transform &trans_a,
    const Com::HitBox &hb_a, const Com::Transform &trans_b,
    const Com::HitBox &hb_b, Engine::Graphics::Vector2f off_a,
    Engine::Graphics::Vector2f off_b);

}  // namespace Rtype::Client
