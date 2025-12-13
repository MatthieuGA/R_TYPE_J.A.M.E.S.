/**
 * @file RenderingEngine.cpp
 * @brief Implementation of the high-level rendering engine.
 */

#include "rendering/RenderingEngine.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <debug/DebugConfig.hpp>

namespace Engine {
namespace Rendering {

RenderingEngine::RenderingEngine(std::shared_ptr<Video::IVideoModule> plugin)
    : plugin_(std::move(plugin)) {
    if (plugin_) {
        std::cout << "[RenderingEngine] Initialized with plugin: "
                  << plugin_->GetModuleName() << std::endl;
    } else {
        std::cerr << "[RenderingEngine] Warning: Initialized with null plugin!"
                  << std::endl;
    }
}

// ===== Lifecycle =====

bool RenderingEngine::Initialize(
    unsigned int width, unsigned int height, const std::string &title) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] CRITICAL ERROR: Cannot initialize "
                     "with null plugin"
                  << std::endl;
        throw std::runtime_error(
            "RenderingEngine: Cannot initialize with null plugin");
    }

    // Initialize camera with window size
    camera_.size =
        Vector2f(static_cast<float>(width), static_cast<float>(height));

    bool success = plugin_->Initialize(width, height, title);
    if (!success) {
        std::cerr
            << "[RenderingEngine] CRITICAL ERROR: Plugin initialization failed"
            << std::endl;
        throw std::runtime_error(
            "RenderingEngine: Plugin initialization failed");
    }

    return success;
}

void RenderingEngine::Shutdown() {
    if (plugin_) {
        plugin_->Shutdown();
    }
}

void RenderingEngine::Update(float delta_time) {
    accumulated_time_ += delta_time;
    if (plugin_) {
        plugin_->Update(delta_time);
    }
}

// ===== Window Management =====

bool RenderingEngine::IsInitialized() const {
    return plugin_ && plugin_->IsInitialized();
}

bool RenderingEngine::IsWindowOpen() const {
    return plugin_ && plugin_->IsWindowOpen();
}

void RenderingEngine::CloseWindow() {
    if (plugin_) {
        plugin_->CloseWindow();
    }
}

Vector2f RenderingEngine::GetWindowSize() const {
    if (!plugin_) {
        return {0.0f, 0.0f};
    }
    return plugin_->GetWindowSize();
}

void RenderingEngine::SetWindowTitle(const std::string &title) {
    if (plugin_) {
        plugin_->SetWindowTitle(title);
    }
}

// ===== Event Handling =====

bool RenderingEngine::PollEvent(Video::Event &event) {
    if (!plugin_) {
        return false;
    }
    return plugin_->PollEvent(event);
}

// ===== Frame Management =====

void RenderingEngine::BeginFrame(const Color &clear_color) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] CRITICAL ERROR: Cannot begin frame - "
                     "plugin is null"
                  << std::endl;
        throw std::runtime_error(
            "RenderingEngine: Cannot begin frame with null plugin");
    }
    plugin_->Clear(clear_color);
}

void RenderingEngine::EndFrame() {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] CRITICAL ERROR: Cannot end frame - "
                     "plugin is null"
                  << std::endl;
        throw std::runtime_error(
            "RenderingEngine: Cannot end frame with null plugin");
    }
    plugin_->Display();
}

// ===== Camera =====

void RenderingEngine::SetCamera(const Camera &camera) {
    camera_ = camera;
}

const Camera &RenderingEngine::GetCamera() const {
    return camera_;
}

Camera &RenderingEngine::GetCamera() {
    return camera_;
}

// ===== High-Level Entity Rendering =====

void RenderingEngine::RenderSprite(const std::string &texture_id,
    const Vector2f &world_position, const Vector2f &world_scale,
    float rotation, const FloatRect *texture_rect, const Color &color,
    const Vector2f &origin_offset, const std::string *shader_id) {
    if (!plugin_) {
        std::cerr
            << "[RenderingEngine] ERROR: Cannot render sprite - plugin is null"
            << std::endl;
        return;
    }

    // Apply camera transformation to convert world to screen coordinates
    Vector2f screen_position = camera_.WorldToScreen(world_position);

    // Apply camera zoom to scale (supports non-uniform scaling and flipping)
    Vector2f final_scale(
        world_scale.x * camera_.zoom, world_scale.y * camera_.zoom);

    // Build transform for rendering
    Transform render_transform;
    render_transform.position = screen_position;
    render_transform.rotation = rotation;
    render_transform.scale = final_scale;
    render_transform.origin = Vector2f(-origin_offset.x, -origin_offset.y);

    // Draw using the plugin
    plugin_->DrawSprite(
        texture_id, render_transform, texture_rect, color, shader_id);

    // Track statistics
    stats_.sprite_draw_calls++;
}

void RenderingEngine::RenderText(const std::string &text,
    const std::string &font_id, const Vector2f &world_position,
    float world_scale, float rotation, unsigned int character_size,
    const Color &color, const Vector2f &origin_offset) {
    if (!plugin_) {
        std::cerr
            << "[RenderingEngine] ERROR: Cannot render text - plugin is null"
            << std::endl;
        return;
    }

    // Apply camera transformation to convert world to screen coordinates
    Vector2f screen_position = camera_.WorldToScreen(world_position);

    // Apply camera zoom to scale
    float final_scale = world_scale * camera_.zoom;

    // Build transform for text rendering
    Transform render_transform;
    render_transform.position = screen_position;
    render_transform.rotation = rotation;
    render_transform.scale = Vector2f(final_scale, final_scale);
    render_transform.origin = Vector2f(-origin_offset.x, -origin_offset.y);

    // Draw text using the plugin
    plugin_->DrawText(text, font_id, render_transform, character_size, color);

    // Track statistics
    stats_.text_draw_calls++;
}

void RenderingEngine::RenderParticles(const std::vector<Vector2f> &particles,
    const std::vector<Color> &colors, const std::vector<float> &sizes,
    int z_index) {
    DEBUG_PARTICLES_LOG(
        "RenderParticles called with " << particles.size() << " particles");

    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot render particles - "
                     "plugin is null"
                  << std::endl;
        return;
    }

    if (particles.empty()) {
        DEBUG_PARTICLES_LOG("Early return (empty particle list)");
        return;
    }

    DEBUG_PARTICLES_LOG(
        "Creating vertex array for " << particles.size() << " particles");

    // Use vertex array for efficient batched rendering
    // Each particle is rendered as a small quad (4 vertices per particle)
    std::vector<Video::Vertex> vertices;
    vertices.reserve(particles.size() * 4);

    DEBUG_PARTICLES_LOG("Reserved " << (particles.size() * 4) << " vertices");

    for (size_t i = 0; i < particles.size(); ++i) {
        const float half_size = ((i < sizes.size()) ? sizes[i] : 2.0f) / 2.0f;
        const Color &color =
            (i < colors.size()) ? colors[i] : Color(255, 255, 255, 255);
        const Vector2f &pos = particles[i];

        // Create a quad (4 vertices) for each particle
        Video::Vertex v1, v2, v3, v4;
        v1.position = Vector2f(pos.x - half_size, pos.y - half_size);
        v2.position = Vector2f(pos.x + half_size, pos.y - half_size);
        v3.position = Vector2f(pos.x + half_size, pos.y + half_size);
        v4.position = Vector2f(pos.x - half_size, pos.y + half_size);

        v1.color = v2.color = v3.color = v4.color = color;

        // Add all 4 vertices for the quad
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
    }

    DEBUG_PARTICLES_LOG("Built " << vertices.size() << " vertices");

    // Draw all particles in a single batch using Quads primitive
    Video::RenderStates states;

    DEBUG_PARTICLES_LOG(
        "Calling DrawVertices with " << vertices.size() << " vertices");

    plugin_->DrawVertices(
        vertices.data(), vertices.size(), 3, states);  // 3 = Quads

    DEBUG_PARTICLES_LOG("DrawVertices completed");

    // Track statistics
    stats_.particle_batches++;
    stats_.total_particles += particles.size();
}

// ===== Resource Management =====

bool RenderingEngine::LoadTexture(
    const std::string &id, const std::string &path) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot load texture '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }

    // Increment reference count
    texture_ref_counts_[id]++;

    // Only load if this is the first reference
    if (texture_ref_counts_[id] == 1) {
        bool success = plugin_->LoadTexture(id, path);
        if (!success) {
            std::cerr << "[RenderingEngine] ERROR: Failed to load texture '"
                      << id << "' from '" << path << "'" << std::endl;
            // Decrement ref count on failure
            texture_ref_counts_[id]--;
            if (texture_ref_counts_[id] == 0) {
                texture_ref_counts_.erase(id);
            }
        }
        return success;
    }

    // Already loaded, just return success
    return true;
}

bool RenderingEngine::UnloadTexture(const std::string &id) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot unload texture '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }

    // Find texture in reference count map
    auto it = texture_ref_counts_.find(id);
    if (it == texture_ref_counts_.end() || it->second == 0) {
        // Not loaded or already fully unloaded
        return false;
    }

    // Decrement reference count
    it->second--;

    // Only unload from plugin when reference count reaches zero
    if (it->second == 0) {
        texture_ref_counts_.erase(it);
        // Call plugin to actually free GPU memory
        return plugin_->UnloadTexture(id);
    }

    return true;
}

bool RenderingEngine::LoadFont(
    const std::string &id, const std::string &path) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot load font '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }
    bool success = plugin_->LoadFont(id, path);
    if (!success) {
        std::cerr << "[RenderingEngine] ERROR: Failed to load font '" << id
                  << "' from '" << path << "'" << std::endl;
    }
    return success;
}

bool RenderingEngine::UnloadFont(const std::string &id) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot unload font '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }
    return plugin_->UnloadFont(id);
}

bool RenderingEngine::LoadShader(const std::string &id,
    const std::string &vertex_path, const std::string &fragment_path) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot load shader '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }
    bool success = plugin_->LoadShader(id, vertex_path, fragment_path);
    if (!success) {
        std::cerr << "[RenderingEngine] ERROR: Failed to load shader '" << id
                  << "' (vertex: '" << vertex_path << "', fragment: '"
                  << fragment_path << "')" << std::endl;
    }
    return success;
}

bool RenderingEngine::UnloadShader(const std::string &id) {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot unload shader '" << id
                  << "' - plugin is null" << std::endl;
        return false;
    }
    return plugin_->UnloadShader(id);
}

Vector2f RenderingEngine::GetTextureSize(const std::string &id) const {
    if (!plugin_) {
        std::cerr << "[RenderingEngine] ERROR: Cannot get texture size for '"
                  << id << "' - plugin is null" << std::endl;
        return Vector2f(0.0f, 0.0f);
    }
    return plugin_->GetTextureSize(id);
}

FloatRect RenderingEngine::GetTextBounds(const std::string &text,
    const std::string &font_id, unsigned int character_size) const {
    if (!plugin_) {
        return FloatRect(0, 0, 0, 0);
    }
    return plugin_->GetTextBounds(text, font_id, character_size);
}

// ===== Shader Management =====

void RenderingEngine::SetShaderParameter(
    const std::string &shader_id, const std::string &name, float value) {
    if (plugin_) {
        plugin_->SetShaderParameter(shader_id, name, value);
    }
}

// ===== Low-Level Primitive Drawing =====

void RenderingEngine::DrawRectangle(const FloatRect &rect, const Color &color,
    const Color *outline_color, float outline_thickness) {
    if (plugin_) {
        plugin_->DrawRectangle(rect, color, outline_color, outline_thickness);
    }
}

void RenderingEngine::DrawCircle(const Vector2f &center, float radius,
    const Color &color, const Color *outline_color, float outline_thickness) {
    if (plugin_) {
        plugin_->DrawCircle(
            center, radius, color, outline_color, outline_thickness);
    }
}

// ===== Metadata =====

std::string RenderingEngine::GetModuleName() const {
    if (!plugin_) {
        return "None";
    }
    return plugin_->GetModuleName();
}

// ===== Direct Plugin Access =====

Video::IVideoModule *RenderingEngine::GetPlugin() const {
    return plugin_.get();
}

}  // namespace Rendering
}  // namespace Engine
