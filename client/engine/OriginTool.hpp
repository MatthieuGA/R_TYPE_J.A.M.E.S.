#pragma once
#include <graphics/Types.hpp>

#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Rtype::Client {
namespace Com = Component;
Engine::Graphics::Vector2f GetOffsetFromTransform(
    const Com::Transform &transform, Engine::Graphics::Vector2f hit_box);
Engine::Graphics::Vector2f GetOffsetFromAnimatedTransform(
    const Com::Transform &transform,
    const Com::AnimatedSprite &animated_sprite);
}  // namespace Rtype::Client
