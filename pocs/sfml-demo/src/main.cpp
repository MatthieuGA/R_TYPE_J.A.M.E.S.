#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "components.hpp"
#include "entity.hpp"
#include "registry.hpp"

constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 600;
constexpr float PLAYER_SPEED = 250.0f;
constexpr float BULLET_SPEED = 400.0f;
constexpr float ENEMY_SPEED = 150.0f;
constexpr float PLAYER_START_X = 50.0f;
constexpr float ENEMY_SPAWN_X = WINDOW_WIDTH + 50.0f;
constexpr float SHOOT_COOLDOWN = 0.3f; // seconds between shots

// Render system - draws all entities with Position + Sprite
void render_system(
    sf::RenderWindow& window,
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Sprite>& sprites,
    ecs::sparse_array<ecs::Bullet>& bullets,
    std::unordered_map<std::string, sf::Texture>& textures,
    std::unordered_map<ecs::entity::id_type, sf::Sprite>& sprite_cache)
{
    for (std::size_t i = 0; i < positions.size() && i < sprites.size(); ++i) {
        if (positions[i] && sprites[i]) {
            const auto& pos = positions[i].value();
            const auto& sprite_comp = sprites[i].value();
            bool is_bullet = i < bullets.size() && bullets[i].has_value();

            // Load texture if not cached
            if (textures.find(sprite_comp.texture_path) == textures.end()) {
                sf::Texture texture;
                if (texture.loadFromFile(sprite_comp.texture_path)) {
                    textures[sprite_comp.texture_path] = std::move(texture);
                } else {
                    std::cerr << "Failed to load texture: " << sprite_comp.texture_path << std::endl;
                    continue;
                }
            }

            // Create or update sprite
            if (sprite_cache.find(i) == sprite_cache.end()) {
                sf::Sprite sf_sprite;
                sf_sprite.setTexture(textures[sprite_comp.texture_path]);
                sprite_cache[i] = sf_sprite;
            }

            auto& sf_sprite = sprite_cache[i];
            sf_sprite.setPosition(pos.x, pos.y);
            
            // Scale sprite to match the desired width and height
            sf::Vector2u texture_size = textures[sprite_comp.texture_path].getSize();
            if (texture_size.x > 0 && texture_size.y > 0) {
                float scale_x = sprite_comp.width / static_cast<float>(texture_size.x);
                float scale_y = sprite_comp.height / static_cast<float>(texture_size.y);
                sf_sprite.setScale(scale_x, scale_y);
            }
            
            // Color bullets yellow
            if (is_bullet) {
                sf_sprite.setColor(sf::Color::Yellow);
            } else {
                sf_sprite.setColor(sf::Color::White);
            }
            
            window.draw(sf_sprite);
        }
    }
}

// Movement system - updates Position based on Velocity
void movement_system(
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Velocity>& velocities,
    float delta_time)
{
    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        if (positions[i] && velocities[i]) {
            auto& pos = positions[i].value();
            const auto& vel = velocities[i].value();
            
            pos.x += vel.dx * delta_time;
            pos.y += vel.dy * delta_time;
        }
    }
}

// Boundary system - keeps player within screen bounds
void boundary_system(
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Player>& players,
    ecs::sparse_array<ecs::Hitbox>& hitboxes)
{
    for (std::size_t i = 0; i < positions.size(); ++i) {
        if (positions[i] && players[i] && hitboxes[i]) {
            auto& pos = positions[i].value();
            const auto& hitbox = hitboxes[i].value();
            
            // Clamp position to screen bounds
            if (pos.x < 0.0f) pos.x = 0.0f;
            if (pos.y < 0.0f) pos.y = 0.0f;
            if (pos.x + hitbox.width > WINDOW_WIDTH) pos.x = WINDOW_WIDTH - hitbox.width;
            if (pos.y + hitbox.height > WINDOW_HEIGHT) pos.y = WINDOW_HEIGHT - hitbox.height;
        }
    }
}

// Cleanup system - removes entities that went off-screen
void cleanup_system(
    ecs::registry& registry,
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Bullet>& bullets,
    ecs::sparse_array<ecs::Enemy>& enemies)
{
    std::vector<ecs::entity> to_kill;
    
    for (std::size_t i = 0; i < positions.size(); ++i) {
        if (positions[i]) {
            const auto& pos = positions[i].value();
            bool is_bullet = i < bullets.size() && bullets[i].has_value();
            bool is_enemy = i < enemies.size() && enemies[i].has_value();
            
            // Remove bullets that went off-screen (right)
            if (is_bullet && pos.x > WINDOW_WIDTH + 50.0f) {
                to_kill.push_back(ecs::entity(i));
            }
            // Remove enemies that went off-screen (left)
            if (is_enemy && pos.x < -100.0f) {
                to_kill.push_back(ecs::entity(i));
            }
        }
    }
    
    for (auto& entity : to_kill) {
        registry.kill_entity(entity);
    }
}

// Collision system - handles bullet-enemy collisions and player-enemy collisions
bool collision_system(
    ecs::registry& registry,
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Hitbox>& hitboxes,
    ecs::sparse_array<ecs::Bullet>& bullets,
    ecs::sparse_array<ecs::Enemy>& enemies,
    ecs::sparse_array<ecs::Player>& players)
{
    std::vector<ecs::entity> to_kill;
    bool game_over = false;
    
    // Check all bullets against all enemies
    for (std::size_t b = 0; b < positions.size(); ++b) {
        if (!positions[b] || !bullets[b] || !hitboxes[b]) continue;
        
        const auto& bullet_pos = positions[b].value();
        const auto& bullet_box = hitboxes[b].value();
        
        for (std::size_t e = 0; e < positions.size(); ++e) {
            if (!positions[e] || !enemies[e] || !hitboxes[e]) continue;
            if (b == e) continue;
            
            const auto& enemy_pos = positions[e].value();
            const auto& enemy_box = hitboxes[e].value();
            
            // AABB collision detection
            bool collision = !(bullet_pos.x + bullet_box.width < enemy_pos.x ||
                             bullet_pos.x > enemy_pos.x + enemy_box.width ||
                             bullet_pos.y + bullet_box.height < enemy_pos.y ||
                             bullet_pos.y > enemy_pos.y + enemy_box.height);
            
            if (collision) {
                to_kill.push_back(ecs::entity(b)); // Kill bullet
                to_kill.push_back(ecs::entity(e)); // Kill enemy
            }
        }
    }
    
    // Check player against all enemies
    for (std::size_t p = 0; p < positions.size(); ++p) {
        if (!positions[p] || !players[p] || !hitboxes[p]) continue;
        
        const auto& player_pos = positions[p].value();
        const auto& player_box = hitboxes[p].value();
        
        for (std::size_t e = 0; e < positions.size(); ++e) {
            if (!positions[e] || !enemies[e] || !hitboxes[e]) continue;
            if (p == e) continue;
            
            const auto& enemy_pos = positions[e].value();
            const auto& enemy_box = hitboxes[e].value();
            
            // AABB collision detection
            bool collision = !(player_pos.x + player_box.width < enemy_pos.x ||
                             player_pos.x > enemy_pos.x + enemy_box.width ||
                             player_pos.y + player_box.height < enemy_pos.y ||
                             player_pos.y > enemy_pos.y + enemy_box.height);
            
            if (collision) {
                game_over = true;
                break;
            }
        }
        
        if (game_over) break;
    }
    
    // Remove duplicates and kill entities
    std::sort(to_kill.begin(), to_kill.end(), 
              [](const ecs::entity& a, const ecs::entity& b) { return a.id() < b.id(); });
    to_kill.erase(std::unique(to_kill.begin(), to_kill.end(),
                              [](const ecs::entity& a, const ecs::entity& b) { return a.id() == b.id(); }), 
                  to_kill.end());
    
    for (auto& entity : to_kill) {
        registry.kill_entity(entity);
    }
    
    return game_over;
}

int main()
{
    // Create window
    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "R-Type PoC - SFML + ECS"
    );
    window.setFramerateLimit(60);

    // Create ECS registry
    ecs::registry registry;
    
    // Register components
    auto& positions = registry.register_component<ecs::Position>();
    auto& velocities = registry.register_component<ecs::Velocity>();
    auto& sprites = registry.register_component<ecs::Sprite>();
    auto& hitboxes = registry.register_component<ecs::Hitbox>();
    auto& players = registry.register_component<ecs::Player>();
    auto& enemies = registry.register_component<ecs::Enemy>();
    auto& bullets = registry.register_component<ecs::Bullet>();

    // Create player entity (spawns on the left)
    auto player = registry.spawn_entity();
    registry.add_component(player, ecs::Position{PLAYER_START_X, WINDOW_HEIGHT / 2.0f});
    registry.add_component(player, ecs::Velocity{0.0f, 0.0f});
    registry.add_component(player, ecs::Sprite{"assets/player.png", 32.0f, 32.0f});
    registry.add_component(player, ecs::Hitbox{32.0f, 32.0f});
    registry.add_component(player, ecs::Player{});

    // Random number generator for enemy spawning
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> height_dist(50.0f, WINDOW_HEIGHT - 80.0f);

    // Texture and sprite cache for rendering
    std::unordered_map<std::string, sf::Texture> texture_cache;
    std::unordered_map<ecs::entity::id_type, sf::Sprite> sprite_cache;

    // Game loop timing
    sf::Clock clock;
    sf::Clock fps_clock;
    sf::Clock enemy_spawn_clock;
    int frame_count = 0;
    float shoot_timer = 0.0f;
    bool space_was_pressed = false;
    bool game_over = false;

    // Font for game over text
    sf::Font font;
    sf::Text game_over_text;
    // Try to load a system font (try multiple common paths)
    bool font_loaded = false;
    std::vector<std::string> font_paths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/gnu-free/FreeSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Bold.ttf"
    };
    for (const auto& path : font_paths) {
        if (font.loadFromFile(path)) {
            font_loaded = true;
            break;
        }
    }
    if (!font_loaded) {
        std::cerr << "Warning: Could not load font, text may not display correctly" << std::endl;
    }
    game_over_text.setFont(font);
    game_over_text.setString("GAME OVER");
    game_over_text.setCharacterSize(72);
    game_over_text.setFillColor(sf::Color::Red);
    game_over_text.setStyle(sf::Text::Bold);
    // Center the text
    sf::FloatRect text_bounds = game_over_text.getLocalBounds();
    game_over_text.setOrigin(text_bounds.width / 2.0f, text_bounds.height / 2.0f);
    game_over_text.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

    std::cout << "R-Type PoC started!" << std::endl;
    std::cout << "Arrow keys: Move | Space: Shoot | ESC: Exit" << std::endl;

    while (window.isOpen()) {
        float delta_time = clock.restart().asSeconds();
        
        // If game over, freeze the game
        if (game_over) {
            delta_time = 0.0f;
        } else {
            shoot_timer -= delta_time;
        }

        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        // Input handling - update player velocity (only if not game over)
        if (!game_over && registry.has_component<ecs::Velocity>(player)) {
            auto& vel = velocities[player.id()].value();
            vel.dx = 0.0f;
            vel.dy = 0.0f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                vel.dy = -PLAYER_SPEED;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                vel.dy = PLAYER_SPEED;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                vel.dx = -PLAYER_SPEED;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                vel.dx = PLAYER_SPEED;
            }
        }

        // Shooting - spawn bullet on space press (only if not game over)
        bool space_is_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        if (!game_over && space_is_pressed && !space_was_pressed && shoot_timer <= 0.0f) {
            if (registry.has_component<ecs::Position>(player)) {
                const auto& player_pos = positions[player.id()].value();
                const auto& player_hitbox = hitboxes[player.id()].value();
                
                // Spawn bullet in front of player
                auto bullet = registry.spawn_entity();
                registry.add_component(bullet, ecs::Position{
                    player_pos.x + player_hitbox.width,
                    player_pos.y + player_hitbox.height / 2.0f - 2.0f
                });
                registry.add_component(bullet, ecs::Velocity{BULLET_SPEED, 0.0f});
                registry.add_component(bullet, ecs::Sprite{"assets/player.png", 16.0f, 4.0f});
                registry.add_component(bullet, ecs::Hitbox{16.0f, 4.0f});
                registry.add_component(bullet, ecs::Bullet{1.0f});
                
                shoot_timer = SHOOT_COOLDOWN;
            }
        }
        space_was_pressed = space_is_pressed;

        // Enemy spawning - spawn enemy every 1.5 seconds (only if not game over)
        if (!game_over && enemy_spawn_clock.getElapsedTime().asSeconds() >= 1.5f) {
            auto enemy = registry.spawn_entity();
            float spawn_y = height_dist(gen);
            
            registry.add_component(enemy, ecs::Position{ENEMY_SPAWN_X, spawn_y});
            registry.add_component(enemy, ecs::Velocity{-ENEMY_SPEED, 0.0f});
            registry.add_component(enemy, ecs::Sprite{"assets/enemy.png", 32.0f, 32.0f});
            registry.add_component(enemy, ecs::Hitbox{32.0f, 32.0f});
            registry.add_component(enemy, ecs::Enemy{});
            
            enemy_spawn_clock.restart();
        }

        // Update systems (only if not game over)
        if (!game_over) {
            movement_system(positions, velocities, delta_time);
            boundary_system(positions, players, hitboxes);
            game_over = collision_system(registry, positions, hitboxes, bullets, enemies, players);
            cleanup_system(registry, positions, bullets, enemies);
            
            if (game_over) {
                std::cout << "GAME OVER!" << std::endl;
            }
        }

        // Render
        window.clear(sf::Color::Black);
        render_system(window, positions, sprites, bullets, texture_cache, sprite_cache);
        
        // Draw game over text if game over
        if (game_over) {
            window.draw(game_over_text);
        }
        
        window.display();

        // FPS counter
        frame_count++;
        if (fps_clock.getElapsedTime().asSeconds() >= 1.0f) {
            std::cout << "FPS: " << frame_count << std::endl;
            frame_count = 0;
            fps_clock.restart();
        }
    }

    return 0;
}
