#pragma once

#include <string>

// Position component
struct Position {
    float x{0.0f};
    float y{0.0f};

    Position() = default;
    Position(float x_, float y_) : x(x_), y(y_) {}
};

// Velocity component
struct Velocity {
    float dx{0.0f};
    float dy{0.0f};

    Velocity() = default;
    Velocity(float dx_, float dy_) : dx(dx_), dy(dy_) {}
};

// Sprite component (graphics library agnostic)
struct Sprite {
    std::string texture_path;
    float width{0.0f};
    float height{0.0f};
    
    Sprite() = default;
    Sprite(const std::string& path, float w = 0.0f, float h = 0.0f)
        : texture_path(path), width(w), height(h) {}
};

// Tag component for player entity
struct Player {
    Player() = default;
};

// Tag component for enemy entities
struct Enemy {
    Enemy() = default;
};

// Tag component for bullet entities
struct Bullet {
    float damage{1.0f};
    
    Bullet() = default;
    explicit Bullet(float dmg) : damage(dmg) {}
};

// Hitbox component for collision detection
struct Hitbox {
    float width{0.0f};
    float height{0.0f};
    
    Hitbox() = default;
    Hitbox(float w, float h) : width(w), height(h) {}
};
