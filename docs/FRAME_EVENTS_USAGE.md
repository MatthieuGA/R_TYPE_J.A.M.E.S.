# Frame Events System Usage

## Overview

The Frame Events system allows you to trigger specific actions (like shooting projectiles) at precise animation frames, instead of using a time-based cooldown. This is useful for synchronizing gameplay actions with animations.

## How It Works

### 1. Configure Frame Events

When creating an enemy with shooting capabilities, add frame events to the `EnemyShootTag` component:

```cpp
// Create enemy entity
auto enemy_entity = reg.SpawnEntity();

// Add components
reg.AddComponent<Component::Transform>(enemy_entity, 
    Component::Transform{100.0f, 100.0f, 0.0f, 2.0f});

// Create EnemyShootTag with shooting parameters
auto enemy_shoot_tag = Component::EnemyShootTag(
    1.0f,                                    // cooldown (fallback if no frame events)
    200.0f,                                  // projectile speed
    10,                                      // damage
    Component::EnemyShootTag::STRAIGHT_LEFT, // shoot type
    sf::Vector2f(-10.0f, 0.0f)              // projectile spawn offset
);

// Add frame event: shoot projectile at frame 5 of "Attack" animation
enemy_shoot_tag.AddFrameEvent("Attack", 5);

// You can add multiple events for different frames/animations
enemy_shoot_tag.AddFrameEvent("Attack", 10);      // Second shot at frame 10
enemy_shoot_tag.AddFrameEvent("SpecialAttack", 3); // Different animation

reg.AddComponent<Component::EnemyShootTag>(enemy_entity, std::move(enemy_shoot_tag));

// Add animated sprite with "Attack" animation
auto anim_sprite = Component::AnimatedSprite(32, 32, 0.1f, true);
anim_sprite.AddAnimation("Attack", "enemies/attack.png", 32, 32, 15, 0.1f, true);
reg.AddComponent<Component::AnimatedSprite>(enemy_entity, std::move(anim_sprite));

reg.AddComponent<Component::EnemyTag>(enemy_entity, Component::EnemyTag{100.0f});
```

### 2. Behavior

- **Frame-based shooting**: When frame events are configured, the system monitors the current animation frame.
- **Automatic triggering**: When the animation reaches the specified frame, the action is triggered once.
- **Reset on loop**: When the animation loops back to frame 0, events are reset and can trigger again.
- **Fallback mode**: If no frame events are configured, the system falls back to cooldown-based shooting.

### 3. Example: Enemy Boss with Multi-shot Attack

```cpp
// Create boss enemy
auto boss_entity = reg.SpawnEntity();

// Configure shoot tag with frame events
auto boss_shoot_tag = Component::EnemyShootTag(
    2.0f,      // fallback cooldown
    300.0f,    // fast projectiles
    25,        // high damage
    Component::EnemyShootTag::STRAIGHT_LEFT,
    sf::Vector2f(-20.0f, 0.0f)
);

// Boss shoots 3 projectiles during attack animation
boss_shoot_tag.AddFrameEvent("Attack", 3);   // First shot
boss_shoot_tag.AddFrameEvent("Attack", 6);   // Second shot
boss_shoot_tag.AddFrameEvent("Attack", 9);   // Third shot

// Special attack with different timing
boss_shoot_tag.AddFrameEvent("SpecialAttack", 5);
boss_shoot_tag.AddFrameEvent("SpecialAttack", 15);

reg.AddComponent<Component::EnemyShootTag>(boss_entity, std::move(boss_shoot_tag));

// Setup animations
auto anim = Component::AnimatedSprite(64, 64, 0.08f, true);
anim.AddAnimation("Attack", "boss/attack.png", 64, 64, 12, 0.08f, true);
anim.AddAnimation("SpecialAttack", "boss/special.png", 64, 64, 20, 0.08f, false);
reg.AddComponent<Component::AnimatedSprite>(boss_entity, std::move(anim));
```

## API Reference

### `EnemyShootTag::AddFrameEvent()`

```cpp
void AddFrameEvent(const std::string& animation_name, int frame,
    FrameEvent::EventType event_type = FrameEvent::SHOOT_PROJECTILE);
```

**Parameters:**
- `animation_name`: Name of the animation (e.g., "Attack", "SpecialAttack")
- `frame`: Frame number at which to trigger the event (0-indexed)
- `event_type`: Type of event (currently only `SHOOT_PROJECTILE`)

### `FrameEvent` Structure

```cpp
struct FrameEvent {
    enum EventType {
        SHOOT_PROJECTILE
    };

    std::string animation_name;  // Animation name
    int trigger_frame;           // Frame to trigger at
    EventType event_type;        // Event type
    bool triggered;              // Internal state tracking
};
```

## Benefits

1. **Precise timing**: Actions synchronized with animation frames
2. **Visual feedback**: Projectiles spawn exactly when the animation shows it
3. **Multiple shots**: Easy to configure multi-shot attacks during one animation
4. **Flexibility**: Different events for different animations
5. **Backward compatible**: Falls back to cooldown-based shooting if no events configured

## Notes

- Frame numbers are 0-indexed (first frame = 0)
- Events automatically reset when animation loops
- Only triggers once per animation cycle
- Requires `AnimatedSprite` component to function
