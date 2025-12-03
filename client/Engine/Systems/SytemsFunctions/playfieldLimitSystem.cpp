#include <iostream>
#include "Engine/Systems/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void PlayfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags) {
    for (auto &&[i, tranform, player_tag] :
        make_indexed_zipper(transforms, player_tags)) {
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
