#include <gtest/gtest.h>

#include "../client/include/Components/CoreComponents.hpp"

namespace Com = Rtype::Client::Component;

TEST(ClickableComponent, Defaults) {
    Com::Clickable c;
    EXPECT_EQ(c.idleColor, sf::Color::White);
    EXPECT_EQ(c.hoverColor, sf::Color(200, 200, 200));
    EXPECT_EQ(c.clickColor, sf::Color(150, 150, 150));
    EXPECT_FALSE(c.isHovered);
    EXPECT_FALSE(c.isClicked);
}

TEST(ClickableComponent, OnClickIsInvoked) {
    bool called = false;
    Com::Clickable c;
    c.onClick = [&called]() { called = true; };

    ASSERT_TRUE(static_cast<bool>(c.onClick));
    c.onClick();
    EXPECT_TRUE(called);
}
