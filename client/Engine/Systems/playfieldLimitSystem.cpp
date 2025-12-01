#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void playfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &playerTags) {
    for (auto &&[i, tranform, playerTag] :
        make_indexed_zipper(transforms, playerTags)) {
        // Update position based on velocity
        if (tranform.x < 0) tranform.x = 0;
        if (tranform.y < 0) tranform.y = 0;
        if (tranform.x > window.getSize().x)
            tranform.x = static_cast<float>(window.getSize().x);
        if (tranform.y > window.getSize().y)
            tranform.y = static_cast<float>(window.getSize().y);
    }
}
}  // namespace Rtype::Client
