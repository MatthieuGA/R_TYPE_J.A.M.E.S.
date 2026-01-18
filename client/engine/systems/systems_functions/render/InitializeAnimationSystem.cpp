#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Initialize a drawable that uses an animated sprite sheet.
 *
 * Marks the drawable as loaded. The backend will handle texture loading
 * via the texture_path field.
 *
 * @param drawable Drawable component to initialize
 * @param animatedSprite AnimatedSprite providing frame info
 * @param transform Transform for origin calculation
 */
void InitializeDrawableAnimated(Com::Drawable &drawable,
    const Com::AnimatedSprite &animatedSprite,
    const Com::Transform &transform) {
    auto it = animatedSprite.animations.find(animatedSprite.currentAnimation);
    if (it == animatedSprite.animations.end())
        return;

    drawable.is_loaded = true;
}

/**
 * @brief System to initialize drawables for entities that have an
 * AnimatedSprite component.
 *
 * Ensures textures are loaded and the sprite rectangle/origin are set for
 * animated sprites.
 *
 * @param reg Engine registry (unused)
 * @param transforms Sparse array of Transform components
 * @param drawables Sparse array of Drawable components
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    // Else draw entities with Transform and Drawable components
    for (auto &&[i, transform, drawable, animated_sprite] :
        make_indexed_zipper(transforms, drawables, animated_sprites)) {
        if (!drawable.is_loaded)
            InitializeDrawableAnimated(drawable, animated_sprite, transform);
    }
}

/**
 * @brief Initialize a static (non-animated) drawable.
 *
 * For sprites without AnimatedSprite component, we need to ensure
 * current_rect is initialized. Since we don't have texture size here,
 * we set current_rect to (0,0,0,0) which signals the backend to use
 * the full texture, and we'll get the size from the backend on first draw.
 *
 * @param drawable Drawable component to initialize
 */
void InitializeDrawableStatic(Com::Drawable &drawable) {
    drawable.is_loaded = true;
    // Set current_rect to zero dimensions - backend will use full texture
    drawable.current_rect = Engine::Graphics::IntRect(0, 0, 0, 0);
}

/**
 * @brief System to initialize drawables for entities without AnimatedSprite.
 *
 * For static sprites (buttons, backgrounds), we mark them as loaded.
 * The origin calculation will need texture size, which we'll handle in
 * DrawableSystem by querying the backend.
 *
 * @param reg Engine registry (unused)
 * @param transforms Sparse array of Transform components
 * @param drawables Sparse array of Drawable components
 * @param animated_sprites Sparse array of AnimatedSprite components
 */
void InitializeDrawableStaticSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
    for (auto &&[i, transform, drawable] :
        make_indexed_zipper(transforms, drawables)) {
        if (!drawable.is_loaded && !animated_sprites.has(i)) {
            InitializeDrawableStatic(drawable);
        }
    }
}
}  // namespace Rtype::Client
