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
    int id_player = 0;
    float shoot_cooldown = 0.0f;
    float charge_time = 0.0f;
};

struct AnimationEnterPlayer {
    bool isEntering = true;
};

struct EnemyTag {
    float speed = 100.0f;
};

/**
 * @brief Component to identify the type of enemy.
 *
 * Used to trigger enemy-specific behaviors like death sounds.
 */
struct EnemyType {
    std::string type; /**< Type identifier (e.g., "mermaid", "kamifish") */

    explicit EnemyType(std::string enemy_type = "unknown")
        : type(std::move(enemy_type)) {}
};

struct TimedEvents {
    struct CooldownAction {
        std::function<void(int entity_id)> action;
        float cooldown_max = 1.0f;
        float cooldown = 0.0f;
    };

    std::vector<CooldownAction> cooldown_actions = {};

    TimedEvents(std::function<void(int)> action, float cooldown_max);
    TimedEvents() = default;

    /**
     * @brief Sets the cooldown action to be executed when cooldown elapses.
     *
     * @param action Action callback function to execute (receives entity_id)
     */
    void AddCooldownAction(
        std::function<void(int)> action, float cooldown_max);
};

struct FrameEvents {
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

    std::vector<FrameEvent> frame_events;

    FrameEvents() = default;
    FrameEvents(const std::string &animation_name, int frame,
        std::function<void(int)> action);
    explicit FrameEvents(std::vector<FrameEvent> events);

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
};

struct EnemyShootTag {
    float speed_projectile = 200.0f;
    int damage_projectile = 10;
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
    int previousHealth = 0;

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

/**
 * @brief Component for defining pattern-based movement for entities.
 *
 * This component allows entities to move according to predefined patterns,
 * such as sine waves, waypoints, or following the player. It includes
 * parameters for controlling the movement behavior and state.
 */
struct PatternMovement {
    enum class PatternType {
        Straight,          // Straight line
        SineHorizontal,    // Vertical oscillation
        SineVertical,      // Horizontal oscillation
        ZigZagHorizontal,  // Horizontal zig-zag
        ZigZagVertical,    // Vertical zig-zag
        Wave,              // Wavy movement
        Waypoints,         // Follow predefined waypoints
        FollowPlayer,      // Follow the player
        Circular,          // Circular motion
    };

    // Constructor for Straight movement
    explicit PatternMovement()
        : currentWaypoint(0),
          type(PatternType::Straight),
          elapsed(0.f),
          spawnPos({0.f, 0.f}),
          baseDir({1.f, 0.f}),
          baseSpeed(0.f),
          amplitude({0.f, 0.f}),
          frequency({0.f, 0.f}),
          waypoints({}),
          waypointSpeed(0.f),
          waypointThreshold(4.f),
          targetEntityId(0) {}

    // Constructor for Sine / Wave movement
    PatternMovement(PatternType type, sf::Vector2f amplitude,
        sf::Vector2f frequency, sf::Vector2f baseDir, float baseSpeed,
        bool loop = true)
        : currentWaypoint(0),
          type(type),
          elapsed(0.f),
          spawnPos({0.f, 0.f}),
          baseDir(baseDir),
          baseSpeed(baseSpeed),
          amplitude(amplitude),
          frequency(frequency),
          waypoints({}),
          waypointSpeed(0.f),
          waypointThreshold(4.f),
          targetEntityId(0) {}

    // Constructor for Waypoints movement
    PatternMovement(std::vector<sf::Vector2f> waypoints, sf::Vector2f baseDir,
        float baseSpeed, std::size_t currentWaypoint = 0, bool loop = true)
        : type(PatternType::Waypoints),
          elapsed(0.f),
          spawnPos({0.f, 0.f}),
          baseDir(baseDir),
          baseSpeed(baseSpeed),
          amplitude({0.f, 0.f}),
          frequency({0.f, 0.f}),
          waypoints(std::move(waypoints)),
          currentWaypoint(currentWaypoint),
          waypointThreshold(4.f),
          targetEntityId(0) {}

    // Constructor for Follow Player movement
    explicit PatternMovement(float baseSpeed)
        : currentWaypoint(0),
          type(PatternType::FollowPlayer),
          elapsed(0.f),
          spawnPos({0.f, 0.f}),
          baseDir({0.f, 0.f}),
          baseSpeed(baseSpeed),
          amplitude({0.f, 0.f}),
          frequency({0.f, 0.f}),
          waypoints({}),
          waypointSpeed(0.f),
          waypointThreshold(4.f) {}

    // Constructor for Circular movement
    PatternMovement(float baseSpeed, float radius, sf::Vector2f centerPos)
        : currentWaypoint(0),
          type(PatternType::Circular),
          elapsed(0.f),
          spawnPos(centerPos),
          baseDir({0.f, 0.f}),
          baseSpeed(baseSpeed),
          amplitude({0.f, 0.f}),
          frequency({0.f, 0.f}),
          waypoints({}),
          angle(0.f),
          radius(radius),
          waypointSpeed(0.f),
          waypointThreshold(4.f),
          targetEntityId(0) {}

    PatternType type;

    // Time
    float elapsed = 0.f;  // Times since pattern started

    // Base movement
    sf::Vector2f spawnPos;  // Spawn position of the entity
    sf::Vector2f baseDir;   // Base movement direction (normalized)
    float baseSpeed = 0.f;  // Base movement speed

    // Sine / Wave parameters
    sf::Vector2f amplitude;  // Amplitude of the sine wave
    sf::Vector2f frequency;  // Frequency of the sine wave

    // Waypoints
    std::vector<sf::Vector2f> waypoints;
    std::size_t currentWaypoint = 0;
    float waypointSpeed = 0.f;
    float waypointThreshold = 4.f;  // Distance to consider waypoint reached

    // Cicular
    float angle = 0.f;
    float radius = 0.f;

    // Follow player / target
    size_t targetEntityId = 0;  // Entity ID of the target to follow
};

struct HealthBar {
    sf::Vector2f offset = {0.f, -10.f};
    float percent = 100.f;
    float percent_delay = 100.f;
    bool is_taking_damage = false;

    float timer_damage = 0.f;

    sf::Sprite green_bar;
    sf::Sprite yellow_bar;
    sf::Sprite foreground_bar;
    sf::Texture green_texture;
    sf::Texture yellow_texture;
    sf::Texture foreground_texture;
    bool is_loaded = false;
};

}  // namespace Rtype::Client::Component
