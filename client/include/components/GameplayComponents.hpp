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
    float shoot_cooldown_max = 1.0f;
    float shoot_cooldown = 0.0f;
    float speed_projectile = 200.0f;
    int damage_projectile = 10;

    enum ShootType {
        STRAIGHT_LEFT,
        AIMED
    } shoot_type = STRAIGHT_LEFT;

    sf::Vector2f offset_shoot_position = sf::Vector2f(0.0f, 0.0f);

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
            std::function<void(int)> act)
            : animation_name(anim_name),
              trigger_frame(frame),
              action(std::move(act)),
              triggered(false) {}
    };

    std::vector<FrameEvent> frame_events;

    std::function<void(int entity_id)> cooldown_action = nullptr;

    explicit EnemyShootTag(float cooldown, float speed = 200.0f,
        int damage = 10, ShootType type = STRAIGHT_LEFT,
        sf::Vector2f offset = sf::Vector2f(0.0f, 0.0f))
        : shoot_cooldown_max(cooldown),
          speed_projectile(speed),
          damage_projectile(damage),
          shoot_type(type),
          offset_shoot_position(offset),
          cooldown_action(nullptr) {}

    /**
     * @brief Add a frame event with a custom action callback.
     *
     * @param animation_name Name of the animation (e.g., "Attack")
     * @param frame Frame number to trigger the action
     * @param action Callback function that receives the entity_id
     *
     * @example
     * enemy_shoot_tag.AddFrameEvent("Attack", 5,
     *     [&reg, &enemy_shoot](int entity_id) {
     *         // Custom action - e.g., create projectile
     *         CreateEnemyProjectile(reg, direction, enemy_shoot, entity_id,
     * transform);
     *     });
     */
    void AddFrameEvent(const std::string &animation_name, int frame,
        std::function<void(int)> action) {
        frame_events.emplace_back(animation_name, frame, std::move(action));
    }

    /**
     * @brief Set the cooldown action callback.
     *
     * When the cooldown expires, this action is executed instead of the
     * default behavior. Useful for customizing cooldown-based shooting
     * (without frame events).
     *
     * @param action Callback function that receives the entity_id
     *
     * @example
     * enemy_shoot_tag.SetCooldownAction([&reg](int entity_id) {
     *     // Custom cooldown action
     *     auto &transform = reg.GetComponent<Component::Transform>(entity_id);
     *     CreateEnemyProjectile(reg, direction, enemy_shoot, entity_id,
     * transform);
     * });
     */
    void SetCooldownAction(std::function<void(int)> action) {
        cooldown_action = std::move(action);
    }
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
