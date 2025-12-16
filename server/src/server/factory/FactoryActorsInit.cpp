#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "server/factory/FactoryActors.hpp"

namespace fs = std::filesystem;

namespace server {

int GetIntFromJson(
    const nlohmann::json &j, const std::string &key, int defaultValue) {
    if (j.contains(key) && j[key].is_number_integer())
        return j[key].get<int>();
    return defaultValue;
}

std::string GetStringFromJson(const nlohmann::json &j, const std::string &key,
    const std::string &defaultValue) {
    if (j.contains(key) && j[key].is_string())
        return j[key].get<std::string>();
    return defaultValue;
}

float GetFloatFromJson(
    const nlohmann::json &j, const std::string &key, float defaultValue) {
    if (j.contains(key) && j[key].is_number_float())
        return j[key].get<float>();
    return defaultValue;
}

vector2f GetVector2fFromJson(const nlohmann::json &j, const std::string &key,
    const vector2f &defaultValue = vector2f(0.f, 0.f)) {
    if (j.contains(key) && j[key].is_object()) {
        float x = defaultValue.x;
        float y = defaultValue.y;
        if (j[key].contains("x") && j[key]["x"].is_number_float())
            x = j[key]["x"].get<float>();
        if (j[key].contains("y") && j[key]["y"].is_number_float())
            y = j[key]["y"].get<float>();
        return vector2f(x, y);
    }
    return defaultValue;
}

nlohmann::json GetSubJsonFromJson(
    const nlohmann::json &j, const std::string &key) {
    if (j.contains(key) && j[key].is_object()) {
        return j[key];
    }
    throw std::runtime_error("Key not found or not an object: " + key);
}

void FactoryActors::InitializeEnemyInfoMap(const std::string &jsonFolderPath) {
    // Iterate through all files in the Levels directory
    for (const auto &entry : fs::directory_iterator(jsonFolderPath)) {
        if (!entry.is_regular_file())
            continue;  // Skip regular files
        std::string name = entry.path().filename().stem().string();
        printf("Loading enemy config: %s\n", name.c_str());
        loadConfigEnemy(entry.path().string(), name);
    }
}

void FactoryActors::loadConfigEnemy(
    const std::string &jsonFilePath, const std::string &name) {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << jsonFilePath << std::endl;
        return;
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();
    nlohmann::json parsed = nlohmann::json::parse(fileContent);

    // Extract values from JSON with defaults
    EnnemyInfo info;
    info.tag = GetStringFromJson(parsed, "tag", "unknown");
    info.health = GetIntFromJson(parsed, "health", 100);
    info.speed = GetFloatFromJson(parsed, "speed", 100.0f);
    info.hitbox = GetVector2fFromJson(parsed, "hitbox");

    // Transform
    try {
        nlohmann::json transform = GetSubJsonFromJson(parsed, "transform");
        info.scale =
            GetVector2fFromJson(transform, "scale", vector2f(1.f, 1.f));
    } catch (...) {
        info.scale = vector2f(1.f, 1.f);
    }
    enemy_info_map_[name] = info;
}
}  // namespace server
