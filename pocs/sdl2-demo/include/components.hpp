#pragma once
#include <string>

// Position component - stores x,y coordinates
struct Position {
    float x;
    float y;
    Position() = default;
    Position(float x_, float y_) : x(x_), y(y_) {}
};

// Velocity component - stores movement speed
struct Velocity {
    float dx;
    float dy;
    Velocity() = default;
    Velocity(float dx_, float dy_) : dx(dx_), dy(dy_) {}
};

// Sprite component - stores texture path and dimensions
struct Sprite {
    std::string texture_path;
    float width;
    float height;
    Sprite() = default;
    Sprite(std::string path, float w, float h) 
        : texture_path(path), width(w), height(h) {}
};

// Hitbox component - for collision detection
struct Hitbox {
    float width;
    float height;
    Hitbox() = default;
    Hitbox(float w, float h) : width(w), height(h) {}
};

// Tag components for entity types
struct Player {};
struct Enemy {};
struct Bullet {
    float damage;
    Bullet() : damage(1.0f) {}
    Bullet(float dmg) : damage(dmg) {}
};
