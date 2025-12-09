#pragma once
#include <SFML/Graphics.hpp>

namespace Rtype::Client::Component {
struct PlayerTag {
    float speed_max = 300.0f;
    float shoot_cooldown_max = 0.3f;
    float charge_time_min = 0.5f;
    bool isInPlay = true;
    float shoot_cooldown = 0.0f;
    float charge_time = 0.0f;
    int playerNumber = 0;
};

struct AnimationEnterPlayer {
    bool isEntering = true;
};

struct EnemyTag {
    float speed = 100.0f;
};

struct EnemyShootTag {
    float shoot_cooldown_max = 1.0f;
    float shoot_cooldown = 0.0f;
    float speed_projectile = 200.0f;
    int damage_projectile = 10;

    enum ShootType {
        STRAIGHT_LEFT,
        AIMED
    } shoot_type = STRAIGHT_LEFT;

    sf::Vector2f offset_shoot_position = sf::Vector2f(0.0f, 0.0f);

    explicit EnemyShootTag(float cooldown, float speed = 200.0f,
        int damage = 10, ShootType type = STRAIGHT_LEFT,
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f))
        : shoot_cooldown_max(cooldown),
          speed_projectile(speed),
          damage_projectile(damage),
          shoot_type(type),
          offset_shoot_position(offset) {}
};

struct Projectile {
    int damage;
    sf::Vector2f direction;
    float speed;
    int ownerId;  // ID of the entity that fired the projectile
    bool isEnemyProjectile = false;
};

struct Health {
    int currentHealth;
    int maxHealth;
    bool invincible;
    float invincibilityDuration = 1.0f;
    float invincibilityTimer = 0.0f;

    explicit Health(int maxHealth = 100)
        : currentHealth(maxHealth),
          maxHealth(maxHealth),
          invincible(false),
          invincibilityDuration(0.0f) {}
};

struct StatsGame {
    int score;
};

struct ParrallaxLayer {
    float scroll_speed;
};

/**
 * @brief Component to track entities that are playing a death animation.
 *
 * When an entity dies, it is marked with this component to play the death
 * animation. Once the animation finishes, the entity is removed from the
 * registry by the DeathAnimationSystem.
 */
struct AnimationDeath {
    bool is_dead = true;
};

}  // namespace Rtype::Client::Component
