#pragma once
#include <string>

#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct PlayerTag {
    float speed_max;
    int playerNumber;
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
