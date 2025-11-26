#include "Engine/initRegisterySystems.hpp"
#include "include/indexed_zipper.hpp"
#include <iostream>

using namespace Engine;

namespace Rtype::Client {
    using namespace Component;
void setDrawableOrigin(Drawable &drawable) {
    sf::Vector2f origin;
    if (drawable.origin == Drawable::CENTER) {
        sf::FloatRect bounds = drawable.sprite.getLocalBounds();
        origin = sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f);
    } else {
        origin = sf::Vector2f(0.0f, 0.0f);
    }
    drawable.sprite.setOrigin(origin);
}

void initializeDrawable(Drawable &drawable) {
    if (!drawable.texture.loadFromFile(drawable.spritePath))
        std::cerr << "ERROR: Failed to load sprite from " << drawable.spritePath << "\n";
    else
        drawable.sprite.setTexture(drawable.texture, true);
    setDrawableOrigin(drawable);
    drawable.isLoaded = true;
}

void drawableSystem(registry &reg, sf::RenderWindow &window,
sparse_array<Transform> const &transforms, sparse_array<Drawable> &drawables) {
    for (auto &&[i, tranform, drawable] :
    make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            initializeDrawable(drawable);

        drawable.sprite.setPosition(sf::Vector2f(tranform.x, tranform.y));
        drawable.sprite.setScale(sf::Vector2f(tranform.scale, tranform.scale));
        const float radians = tranform.rotationDegrees * (3.14159265f / 180.0f);
        drawable.sprite.setRotation(sf::radians(radians));
        window.draw(drawable.sprite);
    }
}
}  // namespace Rtype::Client
