#pragma once
#include "include/Components/CoreComponents.hpp"
#include "include/Components/RenderComponent.hpp"
#include "include/sparse_array.hpp"

namespace Rtype::Client {
namespace Com = Component;
namespace Eng = Engine;

float CalculateCumulativeScale(const Com::Transform &transform,
    const Eng::sparse_array<Com::Transform> &transforms);

sf::Vector2f CalculateWorldPositionWithHierarchy(
    const Com::Transform &transform,
    const Eng::sparse_array<Com::Transform> &transforms);

}  // namespace Rtype::Client
