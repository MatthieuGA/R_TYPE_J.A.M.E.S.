#pragma once
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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
    /**
     * @brief Event to trigger at specific animation frames.
     *
     * Allows executing a custom action callback at a specific frame of an anim
     */
    struct FrameEvent {
        std::string animation_name;
        int trigger_frame;
        std::function<void(int entity_id)> action;
        bool triggered = false;

        /**
         * @brief Constructor for FrameEvent.
         *
         * @param anim_name Name of the animation
         * @param frame Frame number to trigger at
         * @param act Action callback function to execute (receives entity_id)
         */
        FrameEvent(const std::string &anim_name, int frame,
            std::function<void(int)> act);
    };

    struct CooldownAction {
        std::function<void(int entity_id)> action;
        float cooldown_max = 1.0f;
        float cooldown = 0.0f;
    };

    float speed_projectile = 200.0f;
    int damage_projectile = 10;
    std::vector<FrameEvent> frame_events;
    std::vector<CooldownAction> cooldown_actions;
    sf::Vector2f offset_shoot_position = sf::Vector2f(0.0f, 0.0f);

    /**
     * @brief Constructor for EnemyShootTag.
     *
     * @param cooldown Maximum cooldown time between shots
     * @param speed Speed of the projectile
     * @param damage Damage of the projectile
     * @param offset Offset position for shooting
     */
    EnemyShootTag(float speed = 200.0f, int damage = 10,
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f));

    /**
     * @brief Adds a frame event to trigger an action at a specific animation
     * frame.
     *
     * @param animation_name Name of the animation
     * @param frame Frame number to trigger at
     * @param action Action callback function to execute (receives entity_id)
     */
    void AddFrameEvent(const std::string &animation_name, int frame,
        std::function<void(int)> action);

    /**
     * @brief Sets the cooldown action to be executed when cooldown elapses.
     *
     * @param action Action callback function to execute (receives entity_id)
     */
    void AddCooldownAction(
        std::function<void(int)> action, float cooldown_max);
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
