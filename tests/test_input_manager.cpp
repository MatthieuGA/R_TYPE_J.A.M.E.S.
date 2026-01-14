/**
 * @file test_input_manager.cpp
 * @brief Unit tests for the InputManager and Input Abstraction Layer.
 */

#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <utility>

#include "game/GameAction.hpp"
#include "game/GameInputBindings.hpp"
#include "input/IInputBackend.hpp"
#include "input/InputManager.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

using Engine::Input::IInputBackend;
using Engine::Input::InputManager;
using Engine::Input::Key;
using Engine::Input::MouseButton;
using Engine::Input::MousePosition;

/**
 * @brief Mock input backend for testing purposes.
 *
 * Allows tests to control exactly which keys and buttons are "pressed".
 */
class MockInputBackend : public IInputBackend {
 public:
    bool IsKeyPressed(Key key) const override {
        return pressed_keys_.count(key) > 0;
    }

    bool IsMouseButtonPressed(MouseButton button) const override {
        return pressed_mouse_buttons_.count(button) > 0;
    }

    MousePosition GetMousePosition() const override {
        return mouse_pos_;
    }

    MousePosition GetMousePositionInWindow() const override {
        return mouse_pos_;
    }

    bool HasWindowFocus() const override {
        return has_focus_;
    }

    // Test helpers
    void SetKeyPressed(Key key, bool pressed) {
        if (pressed) {
            pressed_keys_.insert(key);
        } else {
            pressed_keys_.erase(key);
        }
    }

    void SetMouseButtonPressed(MouseButton button, bool pressed) {
        if (pressed) {
            pressed_mouse_buttons_.insert(button);
        } else {
            pressed_mouse_buttons_.erase(button);
        }
    }

    void SetMousePosition(int x, int y) {
        mouse_pos_ = {x, y};
    }

    void SetFocus(bool focus) {
        has_focus_ = focus;
    }

 private:
    std::set<Key> pressed_keys_;
    std::set<MouseButton> pressed_mouse_buttons_;
    MousePosition mouse_pos_{0, 0};
    bool has_focus_ = true;
};

// ============================================================================
// InputManager Basic Tests
// ============================================================================

TEST(InputManager, ConstructsWithBackend) {
    auto backend = std::make_unique<MockInputBackend>();
    InputManager<Game::Action> input_manager(std::move(backend));

    // Should construct successfully
    EXPECT_TRUE(input_manager.HasFocus());
}

TEST(InputManager, RespectsFocusState) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    backend_ptr->SetFocus(true);
    EXPECT_TRUE(input_manager.HasFocus());

    backend_ptr->SetFocus(false);
    EXPECT_FALSE(input_manager.HasFocus());
}

TEST(InputManager, IsMouseButtonPressedForwardsToBackend) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Initially no button pressed
    EXPECT_FALSE(input_manager.IsMouseButtonPressed(MouseButton::Left));

    // Press button
    backend_ptr->SetMouseButtonPressed(MouseButton::Left, true);
    EXPECT_TRUE(input_manager.IsMouseButtonPressed(MouseButton::Left));
    EXPECT_FALSE(input_manager.IsMouseButtonPressed(MouseButton::Right));

    // Release button
    backend_ptr->SetMouseButtonPressed(MouseButton::Left, false);
    EXPECT_FALSE(input_manager.IsMouseButtonPressed(MouseButton::Left));
}

// ============================================================================
// Action Binding Tests
// ============================================================================

TEST(InputManager, BindSingleKeyToAction) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Bind Space to Shoot
    input_manager.BindKey(Game::Action::Shoot, Key::Space);

    // Action should be inactive initially
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));

    // Press Space
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Release Space
    backend_ptr->SetKeyPressed(Key::Space, false);
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));
}

TEST(InputManager, BindMultipleKeysToSameAction) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Bind both Space and Left Mouse to Shoot
    input_manager.BindKey(Game::Action::Shoot, Key::Space);
    input_manager.BindMouseButton(Game::Action::Shoot, MouseButton::Left);

    // Press Space
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Release Space, press Left Mouse
    backend_ptr->SetKeyPressed(Key::Space, false);
    backend_ptr->SetMouseButtonPressed(MouseButton::Left, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Both pressed
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));
}

TEST(InputManager, ClearBindingsForAction) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Bind key
    input_manager.BindKey(Game::Action::Shoot, Key::Space);
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Clear bindings
    input_manager.ClearBindings(Game::Action::Shoot);
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));
}

TEST(InputManager, ClearAllBindings) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Bind multiple actions
    input_manager.BindKey(Game::Action::Shoot, Key::Space);
    input_manager.BindKey(Game::Action::MoveUp, Key::W);

    backend_ptr->SetKeyPressed(Key::Space, true);
    backend_ptr->SetKeyPressed(Key::W, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveUp));

    // Clear all
    input_manager.ClearAllBindings();
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::MoveUp));
}

// ============================================================================
// Action Query Tests
// ============================================================================

TEST(InputManager, IsActionActiveReturnsFalseWhenNoFocus) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::Shoot, Key::Space);
    backend_ptr->SetKeyPressed(Key::Space, true);

    // With focus
    backend_ptr->SetFocus(true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Without focus
    backend_ptr->SetFocus(false);
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));
}

TEST(InputManager, GetAxisReturnsZeroWhenNoKeysPressed) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::MoveLeft, Key::A);
    input_manager.BindKey(Game::Action::MoveRight, Key::D);

    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, 0.0f);
}

TEST(InputManager, GetAxisReturnsNegativeWhenNegativePressed) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::MoveLeft, Key::A);
    input_manager.BindKey(Game::Action::MoveRight, Key::D);

    // Press left
    backend_ptr->SetKeyPressed(Key::A, true);
    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, -1.0f);
}

TEST(InputManager, GetAxisReturnsPositiveWhenPositivePressed) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::MoveLeft, Key::A);
    input_manager.BindKey(Game::Action::MoveRight, Key::D);

    // Press right
    backend_ptr->SetKeyPressed(Key::D, true);
    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, 1.0f);
}

TEST(InputManager, GetAxisReturnsCombinedWhenBothPressed) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::MoveLeft, Key::A);
    input_manager.BindKey(Game::Action::MoveRight, Key::D);

    // Press both (should cancel out)
    backend_ptr->SetKeyPressed(Key::A, true);
    backend_ptr->SetKeyPressed(Key::D, true);
    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, 0.0f);
}

TEST(InputManager, GetAxisReturnsZeroWhenNoFocus) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    input_manager.BindKey(Game::Action::MoveLeft, Key::A);
    backend_ptr->SetKeyPressed(Key::A, true);

    // With focus
    backend_ptr->SetFocus(true);
    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, -1.0f);

    // Without focus
    backend_ptr->SetFocus(false);
    axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, 0.0f);
}

// ============================================================================
// Game-Specific Action Tests
// ============================================================================

TEST(GameInputBindings, SetupDefaultBindingsCreatesMovementBindings) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    Game::SetupDefaultBindings(input_manager);

    // Test QZSD layout
    backend_ptr->SetKeyPressed(Key::Z, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveUp));

    backend_ptr->SetKeyPressed(Key::Z, false);
    backend_ptr->SetKeyPressed(Key::S, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveDown));

    backend_ptr->SetKeyPressed(Key::S, false);
    backend_ptr->SetKeyPressed(Key::Q, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveLeft));

    backend_ptr->SetKeyPressed(Key::Q, false);
    backend_ptr->SetKeyPressed(Key::D, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveRight));
}

TEST(GameInputBindings, SetupDefaultBindingsCreatesWASDAlternatives) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    Game::SetupDefaultBindings(input_manager);

    // Test WASD layout
    backend_ptr->SetKeyPressed(Key::W, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveUp));

    backend_ptr->SetKeyPressed(Key::W, false);
    backend_ptr->SetKeyPressed(Key::A, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveLeft));
}

TEST(GameInputBindings, SetupDefaultBindingsCreatesArrowKeyAlternatives) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    Game::SetupDefaultBindings(input_manager);

    // Test arrow keys
    backend_ptr->SetKeyPressed(Key::Up, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveUp));

    backend_ptr->SetKeyPressed(Key::Up, false);
    backend_ptr->SetKeyPressed(Key::Down, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveDown));

    backend_ptr->SetKeyPressed(Key::Down, false);
    backend_ptr->SetKeyPressed(Key::Left, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveLeft));

    backend_ptr->SetKeyPressed(Key::Left, false);
    backend_ptr->SetKeyPressed(Key::Right, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::MoveRight));
}

TEST(GameInputBindings, SetupDefaultBindingsCreatesShootBindings) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    Game::SetupDefaultBindings(input_manager);

    // Test Space for shooting
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Test Left Mouse for shooting
    backend_ptr->SetKeyPressed(Key::Space, false);
    backend_ptr->SetMouseButtonPressed(MouseButton::Left, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));
}

TEST(GameInputBindings, GetAxisWorksWithDefaultBindings) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    Game::SetupDefaultBindings(input_manager);

    // Test horizontal axis
    backend_ptr->SetKeyPressed(Key::D, true);
    float h_axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(h_axis, 1.0f);

    backend_ptr->SetKeyPressed(Key::D, false);
    backend_ptr->SetKeyPressed(Key::Q, true);
    h_axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(h_axis, -1.0f);

    // Test vertical axis
    backend_ptr->SetKeyPressed(Key::Q, false);
    backend_ptr->SetKeyPressed(Key::Z, true);
    float v_axis =
        input_manager.GetAxis(Game::Action::MoveUp, Game::Action::MoveDown);
    EXPECT_FLOAT_EQ(v_axis, -1.0f);

    backend_ptr->SetKeyPressed(Key::Z, false);
    backend_ptr->SetKeyPressed(Key::S, true);
    v_axis =
        input_manager.GetAxis(Game::Action::MoveUp, Game::Action::MoveDown);
    EXPECT_FLOAT_EQ(v_axis, 1.0f);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(InputManager, HandlesInvalidActionIndex) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Query an action without bindings should return false
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Pause));
}

TEST(InputManager, HandlesEmptyBindings) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // No bindings set up, all actions should be inactive
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::MoveUp));

    float axis =
        input_manager.GetAxis(Game::Action::MoveLeft, Game::Action::MoveRight);
    EXPECT_FLOAT_EQ(axis, 0.0f);
}

TEST(InputManager, RebindingOverwritesPreviousBinding) {
    auto backend_ptr = new MockInputBackend();
    auto backend = std::unique_ptr<IInputBackend>(backend_ptr);
    InputManager<Game::Action> input_manager(std::move(backend));

    // Initial binding
    input_manager.BindKey(Game::Action::Shoot, Key::Space);
    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));

    // Clear and rebind to different key
    input_manager.ClearBindings(Game::Action::Shoot);
    input_manager.BindKey(Game::Action::Shoot, Key::Enter);

    backend_ptr->SetKeyPressed(Key::Space, true);
    EXPECT_FALSE(input_manager.IsActionActive(Game::Action::Shoot));

    backend_ptr->SetKeyPressed(Key::Enter, true);
    EXPECT_TRUE(input_manager.IsActionActive(Game::Action::Shoot));
}
