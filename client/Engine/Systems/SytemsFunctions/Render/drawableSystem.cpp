#include <iostream>
#include <vector>
#include <algorithm>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {

void SetDrawableOrigin(
Com::Drawable &drawable, const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromTransform(transform,
        sf::Vector2f(static_cast<float>(drawable.texture.getSize().x),
                    static_cast<float>(drawable.texture.getSize().y)));
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawable(
Com::Drawable &drawable, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableOrigin(drawable, transform);
    drawable.isLoaded = true;
}

void InitializeExtraDrawable(
Com::ExtraDrawable &extraDrawable, const Com::Transform &transform) {
    for (size_t idx = 0; idx < extraDrawable.spritePaths.size(); ++idx) {
        if (!extraDrawable.textures[idx].loadFromFile(extraDrawable.spritePaths[idx])) {
            std::cerr << "ERROR: Failed to load extra sprite from "
                << extraDrawable.spritePaths[idx] << "\n";
        } else {
            extraDrawable.sprites[idx].setTexture(extraDrawable.textures[idx], true);
            // Set origin
            sf::Vector2f origin = GetOffsetFromTransform(transform,
                sf::Vector2f(static_cast<float>(extraDrawable.textures[idx].getSize().x),
                             static_cast<float>(extraDrawable.textures[idx].getSize().y)));
            extraDrawable.sprites[idx].setOrigin(-origin);
            // Set scale
            extraDrawable.sprites[idx].setScale(
                sf::Vector2f(transform.scale, transform.scale));
        }
    }
    extraDrawable.isLoaded = true;
}

void DrawSprite(GameWorld &game_world, sf::Sprite &sprite,
Com::Drawable *drawable, std::optional<Com::Shader*> shaderCompOpt) {
    if (shaderCompOpt.has_value() && (*shaderCompOpt)->isLoaded) {
        ((*shaderCompOpt)->shader)->setUniform("time",
            game_world.total_time_clock_.getElapsedTime().asSeconds());
        game_world.window_.draw(sprite,
            sf::RenderStates((*shaderCompOpt)->shader.get()));
    } else {
        game_world.window_.draw(sprite);
    }
}

void RenderOneEntity(Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::Shader> &shaders,
Eng::sparse_array<Com::ExtraDrawable> &extra_drawables,
GameWorld &game_world, int i) {
    auto &transform = transforms[i];
    auto &drawable = drawables[i];


    std::optional<Com::Shader*> shaderCompOpt = std::nullopt;
    if (shaders.has(i)) shaderCompOpt = &shaders[i].value();

    drawable->sprite.setPosition(sf::Vector2f(transform->x, transform->y));
    drawable->sprite.setScale(
        sf::Vector2f(transform->scale, transform->scale));
    drawable->sprite.setRotation(transform->rotationDegrees);
    drawable->sprite.setColor(sf::Color(255, 255, 255,
        drawable->opacity * 255));
    DrawSprite(game_world, drawable->sprite, &drawable.value(), shaderCompOpt);
    if (extra_drawables.has(i)) {
        auto &extraDrawable = extra_drawables[i];
        if (!extraDrawable->isLoaded)
            InitializeExtraDrawable(extraDrawable.value(), transform.value());
        // Draw extra sprites
        for (size_t idx = 0; idx < extraDrawable->sprites.size(); ++idx) {
            auto &extraSprite = extraDrawable->sprites[idx];
            extraSprite.setPosition(sf::Vector2f(
                transform->x + extraDrawable->offsets[idx].x,
                transform->y + extraDrawable->offsets[idx].y));
            extraSprite.setRotation(transform->rotationDegrees);
            extraSprite.setColor(sf::Color(255, 255, 255,
                static_cast<sf::Uint8>(extraDrawable->opacities[idx] * 255)));

            DrawSprite(game_world, extraSprite, &drawable.value(), shaderCompOpt);
        }
    }
}

void DrawableSystem(
Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::ExtraDrawable> &extra_drawables,
Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    // Else draw entities with Transform and Drawable components
    for (auto &&[i, transform, drawable] :
    make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        draw_order.push_back(i);
    }

    // Sort by z_index
    std::sort(draw_order.begin(), draw_order.end(),
    [&drawables](int a, int b) {
        return drawables[a]->z_index < drawables[b]->z_index;
    });

    for (auto i : draw_order) {
        RenderOneEntity(transforms, drawables, shaders, extra_drawables,
            game_world, i);
    }
}
}  // namespace Rtype::Client
