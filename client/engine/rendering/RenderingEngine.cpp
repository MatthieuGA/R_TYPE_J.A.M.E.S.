/**
 * @file RenderingEngine.cpp
 * @brief Implementation of the high-level rendering engine.
 */

#include "rendering/RenderingEngine.hpp"

#include <iostream>
#include <string>
#include <utility>

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
        std::cerr << "[RenderingEngine] Cannot initialize: null plugin"
                  << std::endl;
        return false;
    }

    // Initialize camera with window size
    camera_.size =
        Vector2f(static_cast<float>(width), static_cast<float>(height));

    return plugin_->Initialize(width, height, title);
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
    if (plugin_) {
        plugin_->Clear(clear_color);
    }
}

void RenderingEngine::EndFrame() {
    if (plugin_) {
        plugin_->Display();
    }
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
    const Vector2f &world_position, float world_scale, float rotation,
    const FloatRect *texture_rect, const Color &color,
    const Vector2f &origin_offset, const std::string *shader_id) {
    if (!plugin_) {
        return;
    }

    // Build transform for rendering
    Transform render_transform;
    render_transform.position = world_position;
    render_transform.rotation = rotation;
    render_transform.scale = Vector2f(world_scale, world_scale);
    render_transform.origin = Vector2f(-origin_offset.x, -origin_offset.y);

    // Draw using the plugin
    plugin_->DrawSprite(
        texture_id, render_transform, texture_rect, color, shader_id);
}

void RenderingEngine::RenderText(const std::string &text,
    const std::string &font_id, const Vector2f &world_position,
    float world_scale, float rotation, unsigned int character_size,
    const Color &color, const Vector2f &origin_offset) {
    if (!plugin_) {
        return;
    }

    // Build transform for text rendering
    Transform render_transform;
    render_transform.position = world_position;
    render_transform.rotation = rotation;
    render_transform.scale = Vector2f(world_scale, world_scale);
    render_transform.origin = Vector2f(-origin_offset.x, -origin_offset.y);

    // Draw text using the plugin
    plugin_->DrawText(text, font_id, render_transform, character_size, color);
}

// ===== Resource Management =====

bool RenderingEngine::LoadTexture(
    const std::string &id, const std::string &path) {
    if (!plugin_) {
        return false;
    }
    return plugin_->LoadTexture(id, path);
}

bool RenderingEngine::LoadFont(
    const std::string &id, const std::string &path) {
    if (!plugin_) {
        return false;
    }
    return plugin_->LoadFont(id, path);
}

bool RenderingEngine::LoadShader(const std::string &id,
    const std::string &vertex_path, const std::string &fragment_path) {
    if (!plugin_) {
        return false;
    }
    return plugin_->LoadShader(id, vertex_path, fragment_path);
}

Vector2f RenderingEngine::GetTextureSize(const std::string &id) const {
    if (!plugin_) {
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

void *RenderingEngine::GetNativeWindow() const {
    if (!plugin_) {
        return nullptr;
    }
    return plugin_->GetNativeWindow();
}

}  // namespace Rendering
}  // namespace Engine
