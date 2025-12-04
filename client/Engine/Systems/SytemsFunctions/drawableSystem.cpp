#include <iostream>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {

void SetDrawableOrigin(Com::Drawable &drawable,
const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromTransform(transform,
        sf::Vector2f(static_cast<float>(drawable.texture.getSize().x),
                    static_cast<float>(drawable.texture.getSize().y)));
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawable(Com::Drawable &drawable,
const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableOrigin(drawable, transform);
    drawable.isLoaded = true;
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, tranform, drawable] :
    make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
                InitializeDrawable(drawable, tranform);
        draw_order.push_back(i);
    }

    // Sort by z_index
    std::sort(draw_order.begin(), draw_order.end(),
    [&drawables](int a, int b) {
        return drawables[a]->z_index < drawables[b]->z_index;
    });

    for (auto i : draw_order) {
        auto &tranform = transforms[i];
        auto &drawable = drawables[i];
        std::optional<Com::Shader*> shaderCompOpt = std::nullopt;
        if (shaders.has(i)) shaderCompOpt = &shaders[i].value();

        drawable->sprite.setPosition(sf::Vector2f(tranform->x, tranform->y));
        drawable->sprite.setScale(sf::Vector2f(tranform->scale, tranform->scale));
        drawable->sprite.setRotation(tranform->rotationDegrees);
        drawable->sprite.setColor(sf::Color(255, 255, 255,
            drawable->opacity * 255));

        if (shaderCompOpt.has_value() && (*shaderCompOpt)->isLoaded) {
            ((*shaderCompOpt)->shader)->setUniform("time",
                static_cast<float>(game_world.total_time_clock_.getElapsedTime().asSeconds()));
            game_world.window_.draw(drawable->sprite,
                sf::RenderStates((*shaderCompOpt)->shader.get()));
        } else {
            game_world.window_.draw(drawable->sprite);
        }
    }
}
}  // namespace Rtype::Client
