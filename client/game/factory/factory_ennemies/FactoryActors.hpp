#pragma once
#include <map>
#include <memory>
#include <string>

#include <SFML/System.hpp>

#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/registry.hpp"
#include "json/json.hpp"

using json = nlohmann::json;

namespace Rtype::Client {

class FactoryActors {
 public:
    enum class EnnemyType {
        MERMAID
    };

    struct EnnemyInfo {
        std::string tag;
        int health;
        float speed;
        std::string spritePath;
        sf::Vector2f hitbox;
        sf::Vector2f offset_healthbar;
        sf::Vector2f scale;
    };

    static FactoryActors &GetInstance() {
        static FactoryActors instance;
        return instance;
    }

    void CreateActor(Engine::entity &entity, Engine::registry &reg,
        std::string const &tag, bool is_local = false);
    void InitializeEnemyInfoMap(const std::string &jsonFolder);

    /**
     * @brief Reset internal counters for a new game.
     * Should be called when transitioning to lobby or starting a new game.
     */
    void ResetForNewGame() { id_player_ = 0; }

 private:
    void loadConfigEnemy(
        const std::string &jsonFilePath, const std::string &name);

    void CreateBasicActor(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);
    void CreatePlayerActor(Engine::entity &entity, Engine::registry &reg,
        EnnemyInfo info, bool is_local);

    void CreateBasicEnnemy(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);
    void CreateMermaidActor(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);

    // Helper to create a projectile for an enemy
    void CreateMermaidProjectile(Engine::registry &reg, sf::Vector2f direction,
        Component::EnemyShootTag &enemy_shoot, int ownerId,
        Component::Transform const &transform);

    std::map<std::string, EnnemyInfo> enemy_info_map_ = {};
    int id_player_ = 0;
};
}  // namespace Rtype::Client
