#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <unordered_map>
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

// RAII wrapper for SDL_Texture
struct TextureDeleter {
    void operator()(SDL_Texture* tex) const {
        if (tex) SDL_DestroyTexture(tex);
    }
};
using TexturePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;

// Render system - draws all entities with Position + Sprite
void render_system(
    SDL_Renderer* renderer,
    ecs::sparse_array<ecs::Position>& positions,
    ecs::sparse_array<ecs::Sprite>& sprites,
    ecs::sparse_array<ecs::Bullet>& bullets,
    std::unordered_map<std::string, TexturePtr>& textures)
{
    for (std::size_t i = 0; i < positions.size() && i < sprites.size(); ++i) {
        if (positions[i] && sprites[i]) {
            const auto& pos = positions[i].value();
            const auto& sprite_comp = sprites[i].value();
            bool is_bullet = i < bullets.size() && bullets[i].has_value();

            // Load texture if not cached
            if (textures.find(sprite_comp.texture_path) == textures.end()) {
                SDL_Surface* surface = IMG_Load(sprite_comp.texture_path.c_str());
                if (!surface) {
                    std::cerr << "Failed to load image: " << sprite_comp.texture_path 
                              << " - " << IMG_GetError() << std::endl;
                    continue;
                }
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                if (!texture) {
                    std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
                    continue;
                }
                textures[sprite_comp.texture_path] = TexturePtr(texture);
            }

            SDL_Texture* texture = textures[sprite_comp.texture_path].get();
            
            // Color bullets yellow
            if (is_bullet) {
                SDL_SetTextureColorMod(texture, 255, 255, 0);  // Yellow
            } else {
                SDL_SetTextureColorMod(texture, 255, 255, 255);  // White
            }

            SDL_Rect dest_rect{
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                static_cast<int>(sprite_comp.width),
                static_cast<int>(sprite_comp.height)
            };

            SDL_RenderCopy(renderer, texture, nullptr, &dest_rect);
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

int main(int argc, char* argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "R-Type PoC - SDL2 + ECS",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

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

    // Texture cache for rendering
    std::unordered_map<std::string, TexturePtr> texture_cache;

    // Load font for game over text
    TTF_Font* font = nullptr;
    std::vector<std::string> font_paths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/gnu-free/FreeSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Bold.ttf"
    };
    for (const auto& path : font_paths) {
        font = TTF_OpenFont(path.c_str(), 72);
        if (font) break;
    }
    if (!font) {
        std::cerr << "Warning: Could not load font: " << TTF_GetError() << std::endl;
    }

    // Game loop timing
    Uint64 last_time = SDL_GetPerformanceCounter();
    Uint64 fps_last_time = last_time;
    Uint64 enemy_spawn_last_time = last_time;
    int frame_count = 0;
    float shoot_timer = 0.0f;
    bool space_was_pressed = false;
    bool game_over = false;
    bool running = true;

    std::cout << "R-Type PoC started!" << std::endl;
    std::cout << "Arrow keys: Move | Space: Shoot | ESC: Exit" << std::endl;

    while (running) {
        // Calculate delta time
        Uint64 current_time = SDL_GetPerformanceCounter();
        float delta_time = static_cast<float>(current_time - last_time) / SDL_GetPerformanceFrequency();
        last_time = current_time;

        // If game over, freeze the game
        if (game_over) {
            delta_time = 0.0f;
        } else {
            shoot_timer -= delta_time;
        }

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        // Input handling - update player velocity (only if not game over)
        if (!game_over && registry.has_component<ecs::Velocity>(player)) {
            const Uint8* keyboard_state = SDL_GetKeyboardState(nullptr);
            auto& vel = velocities[player.id()].value();
            vel.dx = 0.0f;
            vel.dy = 0.0f;

            if (keyboard_state[SDL_SCANCODE_UP]) {
                vel.dy = -PLAYER_SPEED;
            }
            if (keyboard_state[SDL_SCANCODE_DOWN]) {
                vel.dy = PLAYER_SPEED;
            }
            if (keyboard_state[SDL_SCANCODE_LEFT]) {
                vel.dx = -PLAYER_SPEED;
            }
            if (keyboard_state[SDL_SCANCODE_RIGHT]) {
                vel.dx = PLAYER_SPEED;
            }
        }

        // Shooting - spawn bullet on space press (only if not game over)
        const Uint8* keyboard_state = SDL_GetKeyboardState(nullptr);
        bool space_is_pressed = keyboard_state[SDL_SCANCODE_SPACE];
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
        float time_since_enemy_spawn = static_cast<float>(current_time - enemy_spawn_last_time) / SDL_GetPerformanceFrequency();
        if (!game_over && time_since_enemy_spawn >= 1.5f) {
            auto enemy = registry.spawn_entity();
            float spawn_y = height_dist(gen);
            
            registry.add_component(enemy, ecs::Position{ENEMY_SPAWN_X, spawn_y});
            registry.add_component(enemy, ecs::Velocity{-ENEMY_SPEED, 0.0f});
            registry.add_component(enemy, ecs::Sprite{"assets/enemy.png", 32.0f, 32.0f});
            registry.add_component(enemy, ecs::Hitbox{32.0f, 32.0f});
            registry.add_component(enemy, ecs::Enemy{});
            
            enemy_spawn_last_time = current_time;
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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render_system(renderer, positions, sprites, bullets, texture_cache);
        
        // Draw game over text if game over
        if (game_over && font) {
            SDL_Color red = {255, 0, 0, 255};
            SDL_Surface* text_surface = TTF_RenderText_Solid(font, "GAME OVER", red);
            if (text_surface) {
                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_texture) {
                    SDL_Rect text_rect{
                        (WINDOW_WIDTH - text_surface->w) / 2,
                        (WINDOW_HEIGHT - text_surface->h) / 2,
                        text_surface->w,
                        text_surface->h
                    };
                    SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
                SDL_FreeSurface(text_surface);
            }
        }
        
        SDL_RenderPresent(renderer);

        // FPS counter
        frame_count++;
        float time_since_fps = static_cast<float>(current_time - fps_last_time) / SDL_GetPerformanceFrequency();
        if (time_since_fps >= 1.0f) {
            std::cout << "FPS: " << frame_count << std::endl;
            frame_count = 0;
            fps_last_time = current_time;
        }
    }

    // Cleanup
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
