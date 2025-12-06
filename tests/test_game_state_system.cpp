#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "../client/Engine/Systems/initRegistrySystems.hpp"
#include "../client/include/Components/ScenesComponents.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

namespace Rtype::Client {

class TestScene : public Scene_A {
public:
    void InitScene(Eng::registry &) override { ++init_called; }
    void DestroyScene(Eng::registry &) override { ++destroy_called; }

    int init_called = 0;
    int destroy_called = 0;
};


TEST(GameStateSystem, NoTransitionWhenNextEmpty) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto menu_scene = std::make_shared<TestScene>();
    Com::SceneManagement sm;
    sm.current = "menu";
    sm.next = "";
    sm.scenes.insert({"menu", menu_scene});

    scene_managements.insert_at(0, sm);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(menu_scene->destroy_called, 0);
    EXPECT_EQ(menu_scene->init_called, 0);
    ASSERT_TRUE(scene_managements[0].has_value());
    EXPECT_EQ(scene_managements[0]->current, "menu");
    EXPECT_TRUE(scene_managements[0]->next.empty());
}

TEST(GameStateSystem, NoTransitionWhenNextEqualsCurrent) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto menu_scene = std::make_shared<TestScene>();
    Com::SceneManagement sm;
    sm.current = "menu";
    sm.next = "menu";
    sm.scenes.insert({"menu", menu_scene});

    scene_managements.insert_at(0, sm);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(menu_scene->destroy_called, 0);
    EXPECT_EQ(menu_scene->init_called, 0);
    ASSERT_TRUE(scene_managements[0].has_value());
    EXPECT_EQ(scene_managements[0]->current, "menu");
    EXPECT_EQ(scene_managements[0]->next, "menu");
}

TEST(GameStateSystem, TransitionInvokesDestroyAndInit) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto menu_scene = std::make_shared<TestScene>();
    auto game_scene = std::make_shared<TestScene>();

    Com::SceneManagement sm;
    sm.current = "menu";
    sm.next = "game";
    sm.scenes.insert({"menu", menu_scene});
    sm.scenes.insert({"game", game_scene});

    scene_managements.insert_at(0, sm);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(menu_scene->destroy_called, 1);
    EXPECT_EQ(game_scene->init_called, 1);
    ASSERT_TRUE(scene_managements[0].has_value());
    EXPECT_EQ(scene_managements[0]->current, "game");
    EXPECT_TRUE(scene_managements[0]->next.empty());
}

TEST(GameStateSystem, TransitionHandlesMissingCurrentScene) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto game_scene = std::make_shared<TestScene>();

    Com::SceneManagement sm;
    sm.current = "menu";  // Not present in map
    sm.next = "game";
    sm.scenes.insert({"game", game_scene});

    scene_managements.insert_at(0, sm);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(game_scene->init_called, 1);
    ASSERT_TRUE(scene_managements[0].has_value());
    EXPECT_EQ(scene_managements[0]->current, "game");
    EXPECT_TRUE(scene_managements[0]->next.empty());
}

TEST(GameStateSystem, TransitionHandlesMissingNextScene) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto menu_scene = std::make_shared<TestScene>();

    Com::SceneManagement sm;
    sm.current = "menu";
    sm.next = "settings";  // Not present in map
    sm.scenes.insert({"menu", menu_scene});

    scene_managements.insert_at(0, sm);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(menu_scene->destroy_called, 1);
    ASSERT_TRUE(scene_managements[0].has_value());
    EXPECT_EQ(scene_managements[0]->current, "settings");
    EXPECT_TRUE(scene_managements[0]->next.empty());
}

TEST(GameStateSystem, ProcessesMultipleSceneEntries) {
    Eng::registry reg;
    Eng::sparse_array<Com::SceneManagement> scene_managements;

    auto menu_scene = std::make_shared<TestScene>();
    auto game_scene = std::make_shared<TestScene>();
    auto settings_scene = std::make_shared<TestScene>();

    Com::SceneManagement sm0;
    sm0.current = "menu";
    sm0.next = "game";
    sm0.scenes.insert({"menu", menu_scene});
    sm0.scenes.insert({"game", game_scene});

    Com::SceneManagement sm1;
    sm1.current = "settings";
    sm1.next = "settings";  // Should be ignored
    sm1.scenes.insert({"settings", settings_scene});

    scene_managements.insert_at(0, sm0);
    scene_managements.insert_at(2, sm1);

    GameStateSystem(reg, scene_managements);

    EXPECT_EQ(menu_scene->destroy_called, 1);
    EXPECT_EQ(game_scene->init_called, 1);
    EXPECT_EQ(settings_scene->destroy_called, 0);
    EXPECT_EQ(settings_scene->init_called, 0);

    ASSERT_TRUE(scene_managements[0].has_value());
    ASSERT_TRUE(scene_managements[2].has_value());
    EXPECT_EQ(scene_managements[0]->current, "game");
    EXPECT_TRUE(scene_managements[0]->next.empty());
    EXPECT_EQ(scene_managements[2]->current, "settings");
    EXPECT_EQ(scene_managements[2]->next, "settings");
}

}  // namespace Rtype::Client