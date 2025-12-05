#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {

void ChargingShowAssetPlayerSystem(Eng::registry &reg,
Eng::sparse_array<Com::PlayerTag> &player_tags,
Eng::sparse_array<Com::ExtraDrawable> &extra_drawables) {
    for (auto &&[i, player_tag, extra_drawable] :
        make_indexed_zipper(player_tags, extra_drawables)) {
        if (extra_drawable.sprites.size() == 0)
            continue;
        extra_drawable.opacities[0] =
            player_tag.charge_time > player_tag.charge_time_min ? 1.0f : 0.0f;
    }
}
}  // namespace Rtype::Client
