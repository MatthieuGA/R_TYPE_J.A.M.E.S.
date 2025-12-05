#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {

void ChargingShowAssetPlayerSystem(Eng::registry &reg,
Eng::sparse_array<Com::PlayerTag> &player_tags) {
    for (auto &&[i, player_tag] :
        make_indexed_zipper(player_tags)) {
        // Logic to show charging asset based on player_tag.charge_time
    }
}
}  // namespace Rtype::Client
