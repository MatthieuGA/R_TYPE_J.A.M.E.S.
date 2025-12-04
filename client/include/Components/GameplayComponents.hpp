#pragma once
#include <string>

#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct PlayerTag {
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

}  // namespace Rtype::Client::Component
