#include <iostream>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {

void InitializeShader(Com::Drawable &drawable) {
    if (!drawable.shaderPath.empty()) {
        drawable.shader = std::make_shared<sf::Shader>();
        if (!drawable.shader->loadFromFile(drawable.shaderPath,
            sf::Shader::Fragment)) {
            std::cerr << "ERROR: Failed to load shader from "
                << drawable.shaderPath << "\n";
            drawable.shader = nullptr;
        } else {
            drawable.shader->setUniform("texture", sf::Shader::CurrentTexture);
            drawable.shader->setUniform("amplitude", 0.002f);  // à ajuster
            drawable.shader->setUniform("frequency", 20.f);   // à ajuster
            drawable.shader->setUniform("speed", 2.f);        // à ajuster
        }
    }
}

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
    InitializeShader(drawable);
    drawable.isLoaded = true;
}

void SetDrawableAnimationOrigin(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    sf::Vector2f origin = GetOffsetFromAnimatedTransform(transform,
        animatedSprite);
    drawable.sprite.setOrigin(-origin);
}

void InitializeDrawableAnimated(Com::Drawable &drawable,
const Com::AnimatedSprite &animatedSprite, const Com::Transform &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from "
            << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    SetDrawableAnimationOrigin(drawable, animatedSprite, transform);
    InitializeShader(drawable);
    drawable.isLoaded = true;
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> const &transforms,
Eng::sparse_array<Com::Drawable> &drawables,
Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    std::vector<int> draw_order;

    // Draw all entities with Transform / Drawable / AnimatedSprite components
    for (auto &&[i, tranform, drawable, animated_sprite] :
    make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.isLoaded)
            InitializeDrawableAnimated(drawable, animated_sprite, tranform);
    }

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

        drawable->sprite.setPosition(sf::Vector2f(tranform->x, tranform->y));
        drawable->sprite.setScale(sf::Vector2f(tranform->scale, tranform->scale));
        drawable->sprite.setRotation(tranform->rotationDegrees);

        if (!drawable->shaderPath.empty() && drawable->shader) {
            drawable->shader->setUniform("time", static_cast<float>(
                game_world.total_time_clock_.getElapsedTime().asSeconds()));
            game_world.window_.draw(drawable->sprite, drawable->shader.get());
        } else {
            game_world.window_.draw(drawable->sprite);
        }
    }
}
}  // namespace Rtype::Client
