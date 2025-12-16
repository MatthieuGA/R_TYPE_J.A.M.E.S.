#include <gtest/gtest.h>

#include <string>

#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;

TEST(ComponentsCore, TransformAndVelocity) {
    Com::Transform t{10.0f, 20.0f, 45.0f, 1.25f};
    EXPECT_FLOAT_EQ(t.x, 10.0f);
    EXPECT_FLOAT_EQ(t.y, 20.0f);
    EXPECT_FLOAT_EQ(t.rotationDegrees, 45.0f);
    EXPECT_FLOAT_EQ(t.scale.x, 1.25f);
    EXPECT_FLOAT_EQ(t.scale.y, 1.25f);

    Com::Velocity rb{3.0f, -1.5f};
    EXPECT_FLOAT_EQ(rb.vx, 3.0f);
    EXPECT_FLOAT_EQ(rb.vy, -1.5f);
}

TEST(ComponentsCore, DrawableBasics) {
    Com::Drawable d("Logo.png", 5);
    EXPECT_EQ(d.spritePath, std::string("assets/images/Logo.png"));
    EXPECT_EQ(d.z_index, 5);
    EXPECT_FALSE(d.isLoaded);
}

TEST(ComponentsCore, ControllableAndInput) {
    Com::Controllable c{true};
    EXPECT_TRUE(c.isControllable);

    Com::InputState s{true, false, true, false, true};
    EXPECT_TRUE(s.up);
    EXPECT_FALSE(s.down);
    EXPECT_TRUE(s.left);
    EXPECT_FALSE(s.right);
    EXPECT_TRUE(s.shoot);
}

TEST(ComponentsCore, HitBox) {
    Com::HitBox hb{16.0f, 8.0f, true, 1.0f, 2.0f};
    EXPECT_FLOAT_EQ(hb.width, 16.0f);
    EXPECT_FLOAT_EQ(hb.height, 8.0f);
    EXPECT_FLOAT_EQ(hb.offsetX, 1.0f);
    EXPECT_FLOAT_EQ(hb.offsetY, 2.0f);
}

TEST(ComponentsGameplay, TagsAndProjectile) {
    Com::PlayerTag p;
    p.speed_max = 400.f;
    p.shoot_cooldown_max = 0.5f;
    p.charge_time_min = 0.0f;
    p.shoot_cooldown = 0.0f;
    p.charge_time = 0.0f;
    p.id_player = 2;

    EXPECT_EQ(p.id_player, 2);
    EXPECT_FLOAT_EQ(p.speed_max, 400.f);
    EXPECT_FLOAT_EQ(p.shoot_cooldown_max, 0.5f);
    EXPECT_FLOAT_EQ(p.shoot_cooldown, 0.0f);

    Com::EnemyTag e{};

    Com::Projectile proj{12, {0.0f, -1.0f}, 250.0f, 1, true};
    EXPECT_EQ(proj.damage, 12);
    EXPECT_FLOAT_EQ(proj.speed, 250.0f);
    EXPECT_EQ(proj.ownerId, 1);

    Com::Health h(100);
    EXPECT_EQ(h.currentHealth, 100);
    EXPECT_EQ(h.maxHealth, 100);

    Com::StatsGame stats{9000};
    EXPECT_EQ(stats.score, 9000);
}

TEST(ComponentsNetworking, NetworkIdAndInterpolatedPosition) {
    Com::NetworkId id{42};
    EXPECT_EQ(id.id, 42);

    Com::InterpolatedPosition ip{{100.0f, 200.0f}, 4.5f};
    EXPECT_FLOAT_EQ(ip.goalPosition.x, 100.0f);
    EXPECT_FLOAT_EQ(ip.goalPosition.y, 200.0f);
    EXPECT_FLOAT_EQ(ip.speed, 4.5f);
}
