#pragma once
#include <map>
#include <memory>
#include <string>

#include "include/registry.hpp"
#include "json/json.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Vector2f.hpp"

using json = nlohmann::json;

namespace server {

class FactoryActors {
 public:
    enum class EnnemyType {
        MERMAID,
        KAMI_FISH,
        GOLEM
    };

    struct EnnemyInfo {
        std::string tag;
        int health;
        float speed;
        vector2f hitbox;
        vector2f scale;
    };

    static FactoryActors &GetInstance() {
        static FactoryActors instance;
        return instance;
    }

    void CreateActor(Engine::entity &entity, Engine::registry &reg,
        std::string const &tag, vector2f pos, bool is_local = false);
    void InitializeEnemyInfoMap(const std::string &jsonFolder);

 private:
    void loadConfigEnemy(
        const std::string &jsonFilePath, const std::string &name);

    void CreateBasicActor(vector2f pos, Engine::entity &entity,
        Engine::registry &reg, EnnemyInfo info);
    void CreatePlayerActor(Engine::entity &entity, Engine::registry &reg,
        EnnemyInfo info, bool is_local);

    void CreateBasicEnnemy(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);
    void CreateMermaidActor(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);
    void CreateKamiFishActor(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);
    void CreateGolemActor(
        Engine::entity &entity, Engine::registry &reg, EnnemyInfo info);

    std::map<std::string, EnnemyInfo> enemy_info_map_ = {};
    int id_player_ = 0;
};
}  // namespace server
