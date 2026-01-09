#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/Vector2f.hpp"

namespace server::Component {

struct PlayerTag {
    float speed_max = 800.0f;
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
    // Subtype used for network serialization: matches
    // server::network::EntityState::EnemyType (0=Mermaid,1=KamiFish,2=Daemon)
    uint8_t subtype = 0;
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
    vector2f offset_shoot_position = vector2f(0.0f, 0.0f);

    /**
     * @brief Constructor for EnemyShootTag.
     *
     * @param cooldown Maximum cooldown time between shots
     * @param speed Speed of the projectile
     * @param damage Damage of the projectile
     * @param offset Offset position for shooting
     */
    EnemyShootTag(float speed = 200.0f, int damage = 10,
        vector2f offset = vector2f(0.0f, 0.0f));
};

struct Projectile {
    enum class ProjectileType {
        Normal = 0,
        Charged = 1,
        Enemy_Mermaid = 2
    } type = ProjectileType::Normal;
    int damage;
    vector2f direction;
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
    PatternMovement(PatternType type, vector2f amplitude, vector2f frequency,
        vector2f baseDir, float baseSpeed, bool loop = true)
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
    PatternMovement(std::vector<vector2f> waypoints, vector2f baseDir,
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
    PatternMovement(float baseSpeed, float radius, vector2f centerPos)
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
    vector2f spawnPos;      // Spawn position of the entity
    vector2f baseDir;       // Base movement direction (normalized)
    float baseSpeed = 0.f;  // Base movement speed

    // Sine / Wave parameters
    vector2f amplitude;  // Amplitude of the sine wave
    vector2f frequency;  // Frequency of the sine wave

    // Waypoints
    std::vector<vector2f> waypoints;
    std::size_t currentWaypoint = 0;
    float waypointSpeed = 0.f;
    float waypointThreshold = 4.f;  // Distance to consider waypoint reached

    // Cicular
    float angle = 0.f;
    float radius = 0.f;

    // Follow player / target
    size_t targetEntityId = 0;  // Entity ID of the target to follow
};

/**
 * @brief Component that makes an entity explode when it dies.
 *
 * When the entity's Health reaches zero, the system will deal `damage`
 * to all entities with a `Health` component within `radius` units and
 * trigger the entity's `AnimatedSprite` to play the "Attack" animation.
 */
struct ExplodeOnDeath {
    float radius = 64.0f;  /**< Explosion radius in world units */
    int damage = 20;       /**< Damage dealt to nearby entities */
    bool exploded = false; /**< Internal flag to avoid re-triggering */
};

struct AnimatedSprite {
    struct Animation {
        int totalFrames;
        int current_frame;
        float frameDuration;
        bool loop;

        Animation()
            : totalFrames(0),
              current_frame(0),
              frameDuration(0.0f),
              loop(false) {}

        Animation(int totalFrames, float frameDuration, bool loop)
            : totalFrames(totalFrames),
              current_frame(0),
              frameDuration(frameDuration),
              loop(loop) {}
    };

    std::map<std::string, Animation> animations;
    std::string currentAnimation;
    std::vector<std::pair<std::string, int>> animationQueue;

    bool animated = true;
    float elapsedTime = 0.0f;

    AnimatedSprite(
        bool loop = true, int totalFrames = 0, float frameDuration = 0.1f);
    explicit AnimatedSprite(int current_frame);

    /**
     * @brief Add a new animation to the animation map.
     *
     * @param name The name/key for this animation
     * @param path The path to the texture file (relative to assets/images/)
     * @param frameWidth Width of a single frame
     * @param frameHeight Height of a single frame
     * @param totalFrames Total number of frames in the animation
     * @param frameDuration Duration of each frame in seconds
     * @param loop Whether the animation should loop
     * @param first_frame_position Position of the first frame in the
     * spritesheet
     * @param offset Offset to apply to the sprite position when rendering
     */
    void AddAnimation(const std::string &name, int totalFrames,
        float frameDuration, bool loop = true);

    /**
     * @brief Change the current playing animation.
     *
     * @param name The name of the animation to play
     * @param reset If true, reset the animation to frame 0 and elapsed time to
     * 0
     * @param push_to_queue If true, store the currently playing animation to
     * resume later when interrupted
     * @return true if the animation exists and was changed, false otherwise
     */
    bool SetCurrentAnimation(
        const std::string &name, bool reset = true, bool push_to_queue = true);

    /**
     * @brief Get the current animation object.
     *
     * @return Pointer to the current Animation, or nullptr if not found
     */
    Animation *GetCurrentAnimation();

    /**
     * @brief Get the current animation object (const version).
     *
     * @return Const pointer to the current Animation, or nullptr if not found
     */
    const Animation *GetCurrentAnimation() const;

    std::vector<std::string> GetAnimationNames() const;
};

}  // namespace server::Component
