#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "game/SnapshotTracker.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void KillEntitiesSystem(
    Eng::registry &reg, Eng::sparse_array<Com::NetworkId> &network_ids) {
    const int kMaxTickDifference = 3;

    for (auto &&[i, net_id] : make_indexed_zipper(network_ids)) {
        if (SnapshotTracker::GetInstance().GetLastProcessedTick() -
                static_cast<uint32_t>(net_id.last_processed_tick) >
            kMaxTickDifference) {
            reg.KillEntity(reg.EntityFromIndex(i));
        }
    }
}
}  // namespace Rtype::Client
