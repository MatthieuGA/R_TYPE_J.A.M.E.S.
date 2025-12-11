/**
 * @file PluginVideoBackend.cpp
 * @brief Implementation of video plugin adapter.
 */

#include "engine/video/PluginVideoBackend.hpp"

#include <iostream>
#include <string>
#include <utility>

namespace Engine {
namespace Video {

PluginVideoBackend::PluginVideoBackend(std::shared_ptr<IVideoModule> module)
    : module_(std::move(module)) {
    if (module_) {
        std::cout << "[PluginVideoBackend] Initialized with module: "
                  << module_->GetModuleName() << std::endl;
    } else {
        std::cerr << "[PluginVideoBackend] Warning: Initialized with null "
                     "module!"
                  << std::endl;
    }
}

// ===== Lifecycle =====

bool PluginVideoBackend::Initialize(
    unsigned int width, unsigned int height, const std::string &title) {
    if (!module_) {
        std::cerr << "[PluginVideoBackend] Cannot initialize: null module"
                  << std::endl;
        return false;
    }
    return module_->Initialize(width, height, title);
}

void PluginVideoBackend::Shutdown() {
    if (module_) {
        module_->Shutdown();
    }
}

void PluginVideoBackend::Update(float delta_time) {
    if (module_) {
        module_->Update(delta_time);
    }
}

// ===== Window Management =====

bool PluginVideoBackend::IsWindowOpen() const {
    return module_ && module_->IsWindowOpen();
}

void PluginVideoBackend::CloseWindow() {
    if (module_) {
        module_->CloseWindow();
    }
}

Vector2f PluginVideoBackend::GetWindowSize() const {
    if (!module_) {
        return {0.0f, 0.0f};
    }
    return module_->GetWindowSize();
}

void PluginVideoBackend::SetWindowTitle(const std::string &title) {
    if (module_) {
        module_->SetWindowTitle(title);
    }
}

// ===== Event Handling =====

bool PluginVideoBackend::PollEvent(Event &event) {
    if (!module_) {
        return false;
    }
    return module_->PollEvent(event);
}

// ===== Rendering =====

void PluginVideoBackend::Clear(const Color &color) {
    if (module_) {
        module_->Clear(color);
    }
}

void PluginVideoBackend::Display() {
    if (module_) {
        module_->Display();
    }
}

// ===== Resource Management =====

bool PluginVideoBackend::LoadTexture(
    const std::string &id, const std::string &path) {
    if (!module_) {
        return false;
    }
    return module_->LoadTexture(id, path);
}

bool PluginVideoBackend::LoadFont(
    const std::string &id, const std::string &path) {
    if (!module_) {
        return false;
    }
    return module_->LoadFont(id, path);
}

bool PluginVideoBackend::LoadShader(const std::string &id,
    const std::string &vertex_path, const std::string &fragment_path) {
    if (!module_) {
        return false;
    }
    return module_->LoadShader(id, vertex_path, fragment_path);
}

Vector2f PluginVideoBackend::GetTextureSize(const std::string &id) const {
    if (!module_) {
        return Vector2f(0.0f, 0.0f);
    }
    return module_->GetTextureSize(id);
}

FloatRect PluginVideoBackend::GetTextBounds(const std::string &text,
    const std::string &font_id, unsigned int character_size) const {
    if (!module_) {
        return FloatRect(0, 0, 0, 0);
    }
    return module_->GetTextBounds(text, font_id, character_size);
}

// ===== Drawing =====

void PluginVideoBackend::DrawSprite(const std::string &texture_id,
    const Transform &transform, const FloatRect *texture_rect,
    const Color &color, const std::string *shader_id) {
    if (module_) {
        module_->DrawSprite(
            texture_id, transform, texture_rect, color, shader_id);
    }
}

void PluginVideoBackend::DrawText(const std::string &text,
    const std::string &font_id, const Transform &transform,
    unsigned int character_size, const Color &color) {
    if (module_) {
        module_->DrawText(text, font_id, transform, character_size, color);
    }
}

void PluginVideoBackend::DrawRectangle(const FloatRect &rect,
    const Color &color, const Color *outline_color, float outline_thickness) {
    if (module_) {
        module_->DrawRectangle(rect, color, outline_color, outline_thickness);
    }
}

void PluginVideoBackend::DrawCircle(const Vector2f &center, float radius,
    const Color &color, const Color *outline_color, float outline_thickness) {
    if (module_) {
        module_->DrawCircle(
            center, radius, color, outline_color, outline_thickness);
    }
}

// ===== Shader Management =====

void PluginVideoBackend::SetShaderParameter(
    const std::string &shader_id, const std::string &name, float value) {
    if (module_) {
        module_->SetShaderParameter(shader_id, name, value);
    }
}

// ===== Metadata =====

std::string PluginVideoBackend::GetModuleName() const {
    if (!module_) {
        return "None";
    }
    return module_->GetModuleName();
}

// ===== Direct Access =====

IVideoModule *PluginVideoBackend::GetModule() const {
    return module_.get();
}

void *PluginVideoBackend::GetSFMLWindow() const {
    if (!module_) {
        return nullptr;
    }
    return module_->GetNativeWindow();
}

}  // namespace Video
}  // namespace Engine
