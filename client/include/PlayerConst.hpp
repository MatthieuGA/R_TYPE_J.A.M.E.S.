#pragma once

namespace Rtype::Client {

constexpr const float PLAYER_SPEED_MAX = 400.0f;
constexpr const float PLAYER_SHOOT_COOLDOWN = 0.2f;
constexpr const float PLAYER_CHARGE_TIME = 0.5f;
constexpr const int PLAYER_INITIAL_HEALTH = 3;

constexpr const float PLAYER_SPEED_PROJECTILE = 500.0f;
constexpr const int PLAYER_DAMAGE_PROJECTILE = 10;

constexpr const float PLAYER_SPEED_CHARGED_PROJECTILE = 250.0f;
constexpr const int PLAYER_DAMAGE_CHARGED_PROJECTILE = 500;

constexpr const float PLAYER_GATLING_DURATION = 5.0f;
constexpr const float PLAYER_GATLING_SHOOT_INTERVAL = 0.1f;
constexpr const int PLAYER_GATLING_DAMAGE = 5;
constexpr const float PLAYER_GATLING_PROJECTILE_SPEED = 750.0f;

}  // namespace Rtype::Client
