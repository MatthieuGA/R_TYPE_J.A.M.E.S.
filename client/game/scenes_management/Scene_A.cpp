#include "game/scenes_management/Scene_A.hpp"

#include <string>
#include <utility>

#include "engine/audio/AudioManager.hpp"
#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Rtype::Client {
void Scene_A::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Implementation for initializing the scene
}

void Scene_A::DestroyScene(Engine::registry &reg) {
    for (auto &entity : scene_entities_)
        reg.KillEntity(entity);
    scene_entities_.clear();  // Clear stale entity list for next init
}

Engine::entity Scene_A::CreateEntityInScene(Engine::registry &reg) {
    auto entity = reg.SpawnEntity();
    scene_entities_.push_back(entity);
    return entity;
}

Engine::entity Scene_A::CreateButton(Engine::registry &reg,
    GameWorld &gameWorld, const std::string &label, float x, float y,
    std::function<void()> on_click, float scale) {
    auto button_entity = CreateEntityInScene(reg);

    reg.AddComponent<Component::Transform>(button_entity,
        Component::Transform{x, y, 0.0f, scale, Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        button_entity, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    reg.AddComponent<Component::HitBox>(
        button_entity, Component::HitBox{128.f, 32.f});

    // Wrap the callback to play click sound before executing user's callback
    reg.AddComponent<Component::Clickable>(
        button_entity, Component::Clickable{[&gameWorld, on_click]() {
            // Play button click sound
            if (gameWorld.audio_manager_) {
                gameWorld.audio_manager_->PlaySound("button_click");
            }
            // Execute the user's callback
            on_click();
        }});

    reg.AddComponent<Component::Text>(button_entity,
        Component::Text("dogica.ttf", label, 13, LAYER_UI + 1, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, -5.0f)));

    return button_entity;
}

Engine::entity Scene_A::CreateSlider(Engine::registry &reg,
    GameWorld &gameWorld, float x, float y, float width, float min_value,
    float max_value, float initial_value,
    std::function<void(float)> on_value_changed, float scale) {
    // Create slider track (background bar using stretched button.png)
    auto track_entity = CreateEntityInScene(reg);
    // Scale to stretch button.png horizontally to match width and thin it
    float track_scale_x = (width * scale) / 128.0f;
    float track_scale_y = scale * 0.3f;
    reg.AddComponent<Component::Transform>(track_entity,
        Component::Transform{x, y, 0.0f,
            Engine::Graphics::Vector2f(track_scale_x, track_scale_y),
            Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        track_entity, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});

    // Calculate initial knob position based on initial_value
    float value_range = max_value - min_value;
    float normalized_value =
        (initial_value - min_value) / value_range;  // 0.0 to 1.0
    float knob_offset = (normalized_value - 0.5f) * width * scale;

    // Create slider knob (draggable handle)
    auto knob_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        knob_entity, Component::Transform{x + knob_offset, y, 0.0f,
                         scale * 0.5f, Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        knob_entity, Component::Drawable{"ui/sliderball.png", LAYER_UI + 1});
    reg.AddComponent<Component::HitBox>(
        knob_entity, Component::HitBox{44.f, 44.f});

    // Add draggable component for slider interaction
    float track_half_width = (width * scale) / 2.0f;
    Component::Draggable draggable(x - track_half_width, x + track_half_width);

    // Setup drag callbacks
    draggable.on_drag = [&reg, knob_entity, x, width, scale, min_value,
                            max_value,
                            on_value_changed](float drag_x, float drag_y) {
        // Update knob position
        auto &transform = reg.GetComponent<Component::Transform>(knob_entity);
        transform.x = drag_x;

        // Calculate value from position
        float track_half_width = (width * scale) / 2.0f;
        float normalized_pos =
            (drag_x - (x - track_half_width)) / (2.0f * track_half_width);
        float value = min_value + normalized_pos * (max_value - min_value);

        // Invoke callback with new value
        on_value_changed(value);
    };

    draggable.on_drag_end = [&gameWorld](float drag_x, float drag_y) {
        // Play button click sound when slider drag ends
        if (gameWorld.audio_manager_) {
            gameWorld.audio_manager_->PlaySound("button_click");
        }
    };

    reg.AddComponent<Component::Draggable>(knob_entity, std::move(draggable));

    return knob_entity;
}

}  // namespace Rtype::Client
