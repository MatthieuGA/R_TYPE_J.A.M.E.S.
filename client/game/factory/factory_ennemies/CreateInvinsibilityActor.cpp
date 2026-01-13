#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreateInvinsibilityActor(
    Engine::entity &entity, Engine::registry &reg) {
    // Add drawable and animated sprite components
    reg.AddComponent<Component::Drawable>(entity,
        Component::Drawable("powerups/PowerUp_inv.png", LAYER_ACTORS + 1));
}
}  // namespace Rtype::Client
