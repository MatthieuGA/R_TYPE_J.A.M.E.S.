/**
 * @file test_platform_events.cpp
 * @brief Unit tests for the OS Event Abstraction Layer.
 *
 * Tests the platform event interface, OSEvent structure, and mock
 * implementations.
 */

#include <gtest/gtest.h>

#include <queue>

#include "platform/IPlatformEventSource.hpp"
#include "platform/OSEvent.hpp"

namespace {

using Engine::Platform::IPlatformEventSource;
using Engine::Platform::OSEvent;
using Engine::Platform::OSEventType;

/**
 * @brief Mock implementation of IPlatformEventSource for testing.
 *
 * Allows injection of pre-defined events for testing event consumers.
 */
class MockEventSource : public IPlatformEventSource {
 public:
    /**
     * @brief Inject an event into the mock source.
     *
     * @param event The event to inject
     */
    void InjectEvent(const OSEvent &event) {
        events_.push(event);
    }

    /**
     * @brief Poll the next injected event.
     *
     * @param out The event to populate
     * @return true if an event was available, false otherwise
     */
    bool Poll(OSEvent &out) override {
        if (events_.empty()) {
            return false;
        }
        out = events_.front();
        events_.pop();
        return true;
    }

    /**
     * @brief Check if there are pending events.
     *
     * @return true if events are pending, false otherwise
     */
    bool HasPendingEvents() const {
        return !events_.empty();
    }

    /**
     * @brief Clear all pending events.
     */
    void Clear() {
        while (!events_.empty()) {
            events_.pop();
        }
    }

 private:
    std::queue<OSEvent> events_;
};

// ===========================================================================
// Basic Interface Tests
// ===========================================================================

TEST(PlatformEventSource, MockSourcePollsEmptyWhenNoEvents) {
    MockEventSource source;
    OSEvent event;

    EXPECT_FALSE(source.Poll(event));
    EXPECT_FALSE(source.HasPendingEvents());
}

TEST(PlatformEventSource, MockSourcePollsInjectedEvent) {
    MockEventSource source;

    OSEvent injected;
    injected.type = OSEventType::Closed;

    source.InjectEvent(injected);

    EXPECT_TRUE(source.HasPendingEvents());

    OSEvent polled;
    EXPECT_TRUE(source.Poll(polled));
    EXPECT_EQ(polled.type, OSEventType::Closed);
}

TEST(PlatformEventSource, MockSourcePollsMultipleEvents) {
    MockEventSource source;

    OSEvent evt1;
    evt1.type = OSEventType::GainedFocus;
    OSEvent evt2;
    evt2.type = OSEventType::LostFocus;
    OSEvent evt3;
    evt3.type = OSEventType::Closed;

    source.InjectEvent(evt1);
    source.InjectEvent(evt2);
    source.InjectEvent(evt3);

    OSEvent polled;

    EXPECT_TRUE(source.Poll(polled));
    EXPECT_EQ(polled.type, OSEventType::GainedFocus);

    EXPECT_TRUE(source.Poll(polled));
    EXPECT_EQ(polled.type, OSEventType::LostFocus);

    EXPECT_TRUE(source.Poll(polled));
    EXPECT_EQ(polled.type, OSEventType::Closed);

    EXPECT_FALSE(source.Poll(polled));
}

TEST(PlatformEventSource, MockSourceClearRemovesAllEvents) {
    MockEventSource source;

    OSEvent evt;
    evt.type = OSEventType::Closed;
    source.InjectEvent(evt);
    source.InjectEvent(evt);
    source.InjectEvent(evt);

    EXPECT_TRUE(source.HasPendingEvents());

    source.Clear();

    EXPECT_FALSE(source.HasPendingEvents());
    EXPECT_FALSE(source.Poll(evt));
}

// ===========================================================================
// OSEvent Structure Tests
// ===========================================================================

TEST(OSEvent, ClosedEventHasCorrectType) {
    OSEvent event;
    event.type = OSEventType::Closed;

    EXPECT_EQ(event.type, OSEventType::Closed);
}

TEST(OSEvent, ResizedEventStoresSize) {
    OSEvent event;
    event.type = OSEventType::Resized;
    event.size.width = 1920;
    event.size.height = 1080;

    EXPECT_EQ(event.type, OSEventType::Resized);
    EXPECT_EQ(event.size.width, 1920u);
    EXPECT_EQ(event.size.height, 1080u);
}

TEST(OSEvent, KeyPressedEventStoresKeyData) {
    OSEvent event;
    event.type = OSEventType::KeyPressed;
    event.key.code = Engine::Input::Key::Space;
    event.key.alt = false;
    event.key.control = true;
    event.key.shift = false;
    event.key.system = false;

    EXPECT_EQ(event.type, OSEventType::KeyPressed);
    EXPECT_EQ(event.key.code, Engine::Input::Key::Space);
    EXPECT_FALSE(event.key.alt);
    EXPECT_TRUE(event.key.control);
    EXPECT_FALSE(event.key.shift);
    EXPECT_FALSE(event.key.system);
}

TEST(OSEvent, KeyReleasedEventStoresKeyData) {
    OSEvent event;
    event.type = OSEventType::KeyReleased;
    event.key.code = Engine::Input::Key::Escape;
    event.key.alt = true;
    event.key.control = false;
    event.key.shift = true;
    event.key.system = false;

    EXPECT_EQ(event.type, OSEventType::KeyReleased);
    EXPECT_EQ(event.key.code, Engine::Input::Key::Escape);
    EXPECT_TRUE(event.key.alt);
    EXPECT_FALSE(event.key.control);
    EXPECT_TRUE(event.key.shift);
    EXPECT_FALSE(event.key.system);
}

TEST(OSEvent, MouseButtonPressedEventStoresMouseData) {
    OSEvent event;
    event.type = OSEventType::MouseButtonPressed;
    event.mouseButton.button = Engine::Input::MouseButton::Left;
    event.mouseButton.x = 640;
    event.mouseButton.y = 480;

    EXPECT_EQ(event.type, OSEventType::MouseButtonPressed);
    EXPECT_EQ(event.mouseButton.button, Engine::Input::MouseButton::Left);
    EXPECT_EQ(event.mouseButton.x, 640);
    EXPECT_EQ(event.mouseButton.y, 480);
}

TEST(OSEvent, MouseButtonReleasedEventStoresMouseData) {
    OSEvent event;
    event.type = OSEventType::MouseButtonReleased;
    event.mouseButton.button = Engine::Input::MouseButton::Right;
    event.mouseButton.x = 100;
    event.mouseButton.y = 200;

    EXPECT_EQ(event.type, OSEventType::MouseButtonReleased);
    EXPECT_EQ(event.mouseButton.button, Engine::Input::MouseButton::Right);
    EXPECT_EQ(event.mouseButton.x, 100);
    EXPECT_EQ(event.mouseButton.y, 200);
}

TEST(OSEvent, MouseMovedEventStoresPosition) {
    OSEvent event;
    event.type = OSEventType::MouseMoved;
    event.mouseMove.x = 512;
    event.mouseMove.y = 384;

    EXPECT_EQ(event.type, OSEventType::MouseMoved);
    EXPECT_EQ(event.mouseMove.x, 512);
    EXPECT_EQ(event.mouseMove.y, 384);
}

TEST(OSEvent, MouseWheelScrolledEventStoresScrollData) {
    OSEvent event;
    event.type = OSEventType::MouseWheelScrolled;
    event.mouseWheel.delta = 1.5f;
    event.mouseWheel.x = 400;
    event.mouseWheel.y = 300;

    EXPECT_EQ(event.type, OSEventType::MouseWheelScrolled);
    EXPECT_FLOAT_EQ(event.mouseWheel.delta, 1.5f);
    EXPECT_EQ(event.mouseWheel.x, 400);
    EXPECT_EQ(event.mouseWheel.y, 300);
}

TEST(OSEvent, TextEnteredEventStoresUnicode) {
    OSEvent event;
    event.type = OSEventType::TextEntered;
    event.text.unicode = 0x0041;  // 'A'

    EXPECT_EQ(event.type, OSEventType::TextEntered);
    EXPECT_EQ(event.text.unicode, 0x0041u);
}

TEST(OSEvent, FocusEventsHaveCorrectTypes) {
    OSEvent gained;
    gained.type = OSEventType::GainedFocus;

    OSEvent lost;
    lost.type = OSEventType::LostFocus;

    EXPECT_EQ(gained.type, OSEventType::GainedFocus);
    EXPECT_EQ(lost.type, OSEventType::LostFocus);
}

TEST(OSEvent, MouseEnterLeaveEventsHaveCorrectTypes) {
    OSEvent entered;
    entered.type = OSEventType::MouseEntered;

    OSEvent left;
    left.type = OSEventType::MouseLeft;

    EXPECT_EQ(entered.type, OSEventType::MouseEntered);
    EXPECT_EQ(left.type, OSEventType::MouseLeft);
}

// ===========================================================================
// Event Consumption Simulation
// ===========================================================================

TEST(PlatformEventSource, ConsumerProcessesAllEvents) {
    MockEventSource source;

    // Inject multiple events
    OSEvent evt1;
    evt1.type = OSEventType::KeyPressed;
    evt1.key.code = Engine::Input::Key::W;

    OSEvent evt2;
    evt2.type = OSEventType::KeyReleased;
    evt2.key.code = Engine::Input::Key::W;

    OSEvent evt3;
    evt3.type = OSEventType::Closed;

    source.InjectEvent(evt1);
    source.InjectEvent(evt2);
    source.InjectEvent(evt3);

    // Simulate consumer loop
    int event_count = 0;
    OSEvent event;
    while (source.Poll(event)) {
        event_count++;
    }

    EXPECT_EQ(event_count, 3);
    EXPECT_FALSE(source.HasPendingEvents());
}

TEST(PlatformEventSource, ConsumerCanFilterEvents) {
    MockEventSource source;

    // Inject various events
    OSEvent key_event;
    key_event.type = OSEventType::KeyPressed;
    key_event.key.code = Engine::Input::Key::Space;

    OSEvent mouse_event;
    mouse_event.type = OSEventType::MouseMoved;
    mouse_event.mouseMove.x = 100;
    mouse_event.mouseMove.y = 100;

    OSEvent close_event;
    close_event.type = OSEventType::Closed;

    source.InjectEvent(key_event);
    source.InjectEvent(mouse_event);
    source.InjectEvent(close_event);

    // Consumer only processes key events
    int key_count = 0;
    OSEvent event;
    while (source.Poll(event)) {
        if (event.type == OSEventType::KeyPressed ||
            event.type == OSEventType::KeyReleased) {
            key_count++;
        }
    }

    EXPECT_EQ(key_count, 1);
}

// ===========================================================================
// Edge Cases
// ===========================================================================

TEST(PlatformEventSource, MultipleConsecutivePollsReturnFalse) {
    MockEventSource source;
    OSEvent event;

    EXPECT_FALSE(source.Poll(event));
    EXPECT_FALSE(source.Poll(event));
    EXPECT_FALSE(source.Poll(event));
}

TEST(PlatformEventSource, InjectAfterPollWorks) {
    MockEventSource source;

    OSEvent evt1;
    evt1.type = OSEventType::Closed;
    source.InjectEvent(evt1);

    OSEvent polled;
    EXPECT_TRUE(source.Poll(polled));
    EXPECT_FALSE(source.Poll(polled));

    // Inject new event after polling
    OSEvent evt2;
    evt2.type = OSEventType::GainedFocus;
    source.InjectEvent(evt2);

    EXPECT_TRUE(source.Poll(polled));
    EXPECT_EQ(polled.type, OSEventType::GainedFocus);
}

TEST(OSEvent, UnionMembersAccessibleBasedOnType) {
    OSEvent key_event;
    key_event.type = OSEventType::KeyPressed;
    key_event.key.code = Engine::Input::Key::Enter;

    OSEvent mouse_event;
    mouse_event.type = OSEventType::MouseButtonPressed;
    mouse_event.mouseButton.button = Engine::Input::MouseButton::Middle;

    // Both events can exist independently
    EXPECT_EQ(key_event.type, OSEventType::KeyPressed);
    EXPECT_EQ(key_event.key.code, Engine::Input::Key::Enter);

    EXPECT_EQ(mouse_event.type, OSEventType::MouseButtonPressed);
    EXPECT_EQ(
        mouse_event.mouseButton.button, Engine::Input::MouseButton::Middle);
}

}  // namespace
