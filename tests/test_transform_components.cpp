#include <gtest/gtest.h>

#include "../client/include/Components/CoreComponents.hpp"

namespace Com = Rtype::Client::Component;

TEST(TransformComponent, DefaultsAreCenterAndNoParent) {
    Com::Transform t{};
    EXPECT_EQ(t.origin, Com::Transform::CENTER);
    EXPECT_FALSE(t.parent_entity.has_value());
    EXPECT_TRUE(t.children.empty());
    EXPECT_FLOAT_EQ(t.rotationDegrees, 0.0f);
    EXPECT_FLOAT_EQ(t.scale, 0.0f);  // uninitialized float left as-is
}

TEST(TransformComponent, CustomCtorSetsValues) {
    Com::Transform t{10.0f, -5.0f, 30.0f, 2.5f, Com::Transform::TOP_LEFT,
        sf::Vector2f(1.0f, 2.0f), 7};
    EXPECT_FLOAT_EQ(t.x, 10.0f);
    EXPECT_FLOAT_EQ(t.y, -5.0f);
    EXPECT_FLOAT_EQ(t.rotationDegrees, 30.0f);
    EXPECT_FLOAT_EQ(t.scale, 2.5f);
    EXPECT_EQ(t.origin, Com::Transform::TOP_LEFT);
    EXPECT_FLOAT_EQ(t.customOrigin.x, 1.0f);
    EXPECT_FLOAT_EQ(t.customOrigin.y, 2.0f);
    ASSERT_TRUE(t.parent_entity.has_value());
    EXPECT_EQ(t.parent_entity.value(), 7u);
    EXPECT_TRUE(t.children.empty());
}

TEST(TransformComponent, WorldRotationGetter) {
    Com::Transform t{0.0f, 0.0f, 12.5f, 1.0f};
    EXPECT_FLOAT_EQ(t.GetWorldRotation(), 12.5f);
}

TEST(PhysicsComponents, VelocityDefaultsAndCustom) {
    Com::Velocity v{};
    EXPECT_FLOAT_EQ(v.vx, 0.0f);
    EXPECT_FLOAT_EQ(v.vy, 0.0f);
    EXPECT_FLOAT_EQ(v.accelerationX, 0.0f);
    EXPECT_FLOAT_EQ(v.accelerationY, 0.0f);

    Com::Velocity v2{1.0f, -2.0f, 0.5f, -0.5f};
    EXPECT_FLOAT_EQ(v2.vx, 1.0f);
    EXPECT_FLOAT_EQ(v2.vy, -2.0f);
    EXPECT_FLOAT_EQ(v2.accelerationX, 0.5f);
    EXPECT_FLOAT_EQ(v2.accelerationY, -0.5f);
}

TEST(InputComponents, ControllableAndState) {
    Com::Controllable c{true};
    EXPECT_TRUE(c.isControllable);

    Com::InputState s{true, false, true, false, true};
    EXPECT_TRUE(s.up);
    EXPECT_FALSE(s.down);
    EXPECT_TRUE(s.left);
    EXPECT_FALSE(s.right);
    EXPECT_TRUE(s.shoot);
}

TEST(CollisionComponents, HitBoxDefaultsAndScaling) {
    Com::HitBox hb{16.0f, 8.0f, true, 1.0f, 2.0f};
    EXPECT_FLOAT_EQ(hb.width, 16.0f);
    EXPECT_FLOAT_EQ(hb.height, 8.0f);
    EXPECT_TRUE(hb.scaleWithTransform);
    EXPECT_FLOAT_EQ(hb.offsetX, 1.0f);
    EXPECT_FLOAT_EQ(hb.offsetY, 2.0f);

    Com::HitBox hb2{4.0f, 4.0f, false};
    EXPECT_FALSE(hb2.scaleWithTransform);
}

TEST(CollisionComponents, SolidDefaults) {
    Com::Solid s{};
    EXPECT_TRUE(s.isSolid);
    EXPECT_FALSE(s.isLocked);
}

TEST(InputComponents, InputsStateDefaults) {
    Com::Inputs inputs{};
    EXPECT_FLOAT_EQ(inputs.horizontal, 0.0f);
    EXPECT_FLOAT_EQ(inputs.vertical, 0.0f);
    EXPECT_FALSE(inputs.shoot);
    EXPECT_FALSE(inputs.last_shoot_state);
}
