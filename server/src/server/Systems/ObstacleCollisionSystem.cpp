/**
 * @file ObstacleCollisionSystem.cpp
 * @brief System for handling collisions between players and obstacles.
 *
 * This system implements R-Type style obstacle collision:
 * - Players are blocked by obstacles (cannot pass through)
 * - Players are killed if squished against the left edge of the screen
 */

#include <algorithm>
#include <iostream>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/GameWorldDatas.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/CollidingTools.hpp"
#include "server/systems/OriginTool.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

namespace {
/// Minimum X position before player is considered crushed
constexpr float kCrushZoneThreshold = 32.0f;
}  // namespace

/**
 * @brief Handle the death of a player due to crushing.
 *
 * @param reg The registry.
 * @param entity The player entity to kill.
 * @param i The index of the entity in the registry.
 */
void HandleCrushDeath(
    Engine::registry &reg, Engine::entity entity, std::size_t i) {
    std::cout << "[ObstacleCollision] Player at index " << i << " was CRUSHED!"
              << std::endl;

    // Get player info before removing components
    uint8_t player_id = 0;
    int final_score = 0;
    auto &player_tags = reg.GetComponents<Component::PlayerTag>();
    if (player_tags.has(i)) {
        auto &tag = player_tags[i].value();
        player_id = static_cast<uint8_t>(tag.playerNumber);
        final_score = tag.score - 250;  // Death penalty
    }

    // Notify server about player death with ID and score
    if (Server::GetInstance()) {
        Server::GetInstance()->NotifyPlayerDeath(player_id, final_score);
    }

    // Get fresh references after potential resizes
    auto &animated_sprites = reg.GetComponents<Component::AnimatedSprite>();

    // Mark entity with AnimationDeath component to trigger death animation
    reg.AddComponent<Component::AnimationDeath>(
        entity, Component::AnimationDeath{true});

    // Remove gameplay components
    reg.RemoveComponent<Component::Health>(entity);
    reg.RemoveComponent<Component::HitBox>(entity);
    reg.RemoveComponent<Component::PlayerTag>(entity);
    reg.RemoveComponent<Component::TimedEvents>(entity);
    reg.RemoveComponent<Component::FrameEvents>(entity);

    // Play death animation if available - get fresh reference
    auto &anim_sprites_fresh = reg.GetComponents<Component::AnimatedSprite>();
    if (anim_sprites_fresh.has(i)) {
        auto &anim_sprite = anim_sprites_fresh[i];
        anim_sprite->SetCurrentAnimation("Death", true);
        anim_sprite->animated = true;
    } else {
        // No animated sprite, remove entity immediately
        reg.KillEntity(entity);
    }
}

/**
 * @brief Compute and resolve collision between player and obstacle.
 *
 * Pushes the player out of the obstacle. If the player would be pushed
 * past the left edge of the screen, they are crushed (instant death).
 *
 * @param player_transform Transform of the player (will be modified)
 * @param player_hitbox HitBox of the player
 * @param obs_transform Transform of the obstacle
 * @param obs_hitbox HitBox of the obstacle
 * @return true if player was crushed, false otherwise
 */
bool ResolvePlayerObstacleCollision(Component::Transform &player_transform,
    const Component::HitBox &player_hitbox,
    const Component::Transform &obs_transform,
    const Component::HitBox &obs_hitbox) {
    // Calculate scaled hitbox dimensions
    float p_width =
        player_hitbox.width * (player_hitbox.scaleWithTransform
                                      ? std::abs(player_transform.scale.x)
                                      : 1.0f);
    float p_height =
        player_hitbox.height * (player_hitbox.scaleWithTransform
                                       ? std::abs(player_transform.scale.y)
                                       : 1.0f);
    float o_width =
        obs_hitbox.width * (obs_hitbox.scaleWithTransform
                                   ? std::abs(obs_transform.scale.x)
                                   : 1.0f);
    float o_height =
        obs_hitbox.height * (obs_hitbox.scaleWithTransform
                                    ? std::abs(obs_transform.scale.y)
                                    : 1.0f);

    // Get offsets based on origin
    vector2f p_offset =
        GetOffsetFromTransform(player_transform, {p_width, p_height});
    vector2f o_offset =
        GetOffsetFromTransform(obs_transform, {o_width, o_height});

    // Scale offsets
    vector2f scaled_p_off = {p_offset.x * std::abs(player_transform.scale.x),
        p_offset.y * std::abs(player_transform.scale.y)};
    vector2f scaled_o_off = {o_offset.x * std::abs(obs_transform.scale.x),
        o_offset.y * std::abs(obs_transform.scale.y)};

    // Calculate AABB bounds
    float p_min_x = player_transform.x + scaled_p_off.x;
    float p_max_x = p_min_x + p_width;
    float p_min_y = player_transform.y + scaled_p_off.y;
    float p_max_y = p_min_y + p_height;

    float o_min_x = obs_transform.x + scaled_o_off.x;
    float o_max_x = o_min_x + o_width;
    float o_min_y = obs_transform.y + scaled_o_off.y;
    float o_max_y = o_min_y + o_height;

    // Calculate overlap
    float overlap_x = std::min(p_max_x, o_max_x) - std::max(p_min_x, o_min_x);
    float overlap_y = std::min(p_max_y, o_max_y) - std::max(p_min_y, o_min_y);

    if (overlap_x <= 0 || overlap_y <= 0) {
        return false;  // No actual collision
    }

    // Determine separation direction (push player along smallest axis)
    float p_center_x = (p_min_x + p_max_x) / 2.0f;
    float o_center_x = (o_min_x + o_max_x) / 2.0f;
    float p_center_y = (p_min_y + p_max_y) / 2.0f;
    float o_center_y = (o_min_y + o_max_y) / 2.0f;

    if (overlap_x < overlap_y) {
        // Separate horizontally
        float direction = (p_center_x < o_center_x) ? -1.0f : 1.0f;
        float new_x = player_transform.x + direction * overlap_x;

        // Check if player would be pushed past left edge (crushing)
        if (new_x - p_width / 2.0f < kCrushZoneThreshold) {
            return true;  // Player is crushed!
        }

        player_transform.x = new_x;
    } else {
        // Separate vertically
        float direction = (p_center_y < o_center_y) ? -1.0f : 1.0f;
        player_transform.y += direction * overlap_y;
    }

    return false;  // Not crushed
}

/**
 * @brief Information about a crushed player to be processed.
 */
struct CrushedPlayerInfo {
    Engine::entity entity;
    std::size_t index;
};

/**
 * @brief System to handle collisions between players and obstacles.
 *
 * This system checks for AABB collisions between all players and obstacles.
 * When a collision is detected:
 * - The player is pushed out of the obstacle
 * - If the player is pushed against the left screen edge, they die
 *
 * @param reg ECS registry
 * @param transforms Sparse array of Transform components
 * @param hitboxes Sparse array of HitBox components
 * @param player_tags Sparse array of PlayerTag components
 * @param obstacle_tags Sparse array of ObstacleTag components
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void ObstacleCollisionSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::HitBox> const &hitboxes,
    Engine::sparse_array<Component::PlayerTag> const &player_tags,
    Engine::sparse_array<Component::ObstacleTag> const &obstacle_tags,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites) {
    // Collect crushed players to avoid modifying sparse arrays during
    // iteration
    std::vector<CrushedPlayerInfo> crushed_players;

    // Iterate over all players
    for (auto &&[i, p_transform, p_hitbox, p_tag] :
        make_indexed_zipper(transforms, hitboxes, player_tags)) {
        if (!p_tag.isInPlay) {
            continue;  // Skip inactive players
        }

        Engine::entity player_entity = reg.EntityFromIndex(i);
        bool player_crushed = false;

        // Check collision with all obstacles
        for (auto &&[j, o_transform, o_hitbox, o_tag] :
            make_indexed_zipper(transforms, hitboxes, obstacle_tags)) {
            if (i == j) {
                continue;  // Skip self (shouldn't happen, but safety check)
            }

            // Check for collision
            bool colliding =
                IsColliding(p_transform, p_hitbox, o_transform, o_hitbox);

            // Debug logging (first obstacle only)
            static bool logged = false;
            if (!logged && j == 0) {
                std::cout << "[DEBUG] Player pos=(" << p_transform.x << ","
                          << p_transform.y << ") hitbox=" << p_hitbox.width
                          << "x" << p_hitbox.height << " offset=("
                          << p_hitbox.offsetX << "," << p_hitbox.offsetY << ")"
                          << std::endl;
                std::cout << "[DEBUG] Obstacle pos=(" << o_transform.x << ","
                          << o_transform.y << ") hitbox=" << o_hitbox.width
                          << "x" << o_hitbox.height << " offset=("
                          << o_hitbox.offsetX << "," << o_hitbox.offsetY << ")"
                          << std::endl;
                std::cout << "[DEBUG] Colliding=" << colliding << std::endl;
                logged = true;
            }

            if (!colliding) {
                continue;
            }

            // Resolve collision and check for crushing
            bool is_crushed = ResolvePlayerObstacleCollision(
                p_transform, p_hitbox, o_transform, o_hitbox);

            if (is_crushed) {
                crushed_players.push_back(CrushedPlayerInfo{player_entity, i});
                player_crushed = true;
                break;  // Player is dead, stop checking obstacles
            }
        }

        if (player_crushed) {
            continue;  // Move to next player
        }
    }

    // Now process all crushed players after iteration is complete
    for (const auto &crushed : crushed_players) {
        HandleCrushDeath(reg, crushed.entity, crushed.index);
    }
}

}  // namespace server
