#pragma once
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct PlayerTag {
    float speed_max = 300.0f;
    float shoot_cooldown_max = 0.3f;
    float charge_time_min = 0.5f;
    float shoot_cooldown = 0.0f;
    float charge_time = 0.0f;
    int playerNumber = 0;
};

struct EnemyTag {
    enum class EnemyType {
        BASIC,
        ADVANCED,
        BOSS
    } type;
};

struct Projectile {
    float damage;
    float speed;
    int ownerId;  // ID of the entity that fired the projectile
};

struct Health {
    int currentHealth;
    int maxHealth;
    bool invincible;
    float invincibilityDuration;
};

struct StatsGame {
    int score;
};

struct ParrallaxLayer {
    float scroll_speed;
};

}  // namespace Rtype::Client::Component
