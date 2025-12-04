#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {

void createProjectile(Eng::registry &reg, float x, float y, int ownerId) {
    auto projectile_entity = reg.SpawnEntity();
    const float speed = 800.0f;
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{x, y, 0.0f, 3.0f, Com::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable("OriginalRtype/r-typesheet2.gif", 1));
    reg.AddComponent<Component::AnimatedSprite>(projectile_entity,
        Component::AnimatedSprite{24, 32, 10});
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{10.0f, speed, ownerId});
}


void ShootSystem(Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> &transforms,
Eng::sparse_array<Com::Inputs> const &inputs,
Eng::sparse_array<Com::PlayerTag> &player_tags) {
    for (auto &&[i, transform, input, player_tag] :
        make_indexed_zipper(transforms, inputs, player_tags)) {
        if (player_tag.shoot_cooldown > 0.0f)
            player_tag.shoot_cooldown -= game_world.last_delta_;
        if (input.shoot && player_tag.shoot_cooldown <= 0.0f) {
            // Create projectile entity
            player_tag.shoot_cooldown = player_tag.shoot_cooldown_max;
            createProjectile(reg, transform.x, transform.y, i);
        }
    }
}
}  // namespace Rtype::Client
