/**
 * @file WorldGenConfigLoader.hpp
 * @brief Loads and validates WGF files from core and user directories.
 *
 * The WorldGenConfigLoader is responsible for:
 * - Scanning core and user WGF directories
 * - Parsing and validating JSON files
 * - Assigning default values for optional fields
 * - Maintaining UUID to WGF mappings
 * - Detecting and handling duplicate UUIDs
 */
#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "server/worldgen/WorldGenTypes.hpp"

namespace worldgen {

/**
 * @brief Log levels for WorldGen operations.
 */
enum class LogLevel : uint8_t {
    kInfo = 0,
    kWarning = 1,
    kError = 2,
    kFatal = 3
};

/**
 * @brief Callback type for logging messages.
 */
using LogCallback = std::function<void(LogLevel, const std::string &)>;

/**
 * @brief Statistics about loaded WGF files.
 */
struct LoadStatistics {
    int total_files_scanned = 0;
    int core_files_loaded = 0;
    int user_files_loaded = 0;
    int files_skipped = 0;
    int duplicate_uuids = 0;
    int parse_errors = 0;
    int validation_errors = 0;
};

/**
 * @brief Loads and manages WorldGen Frame (WGF) definitions.
 *
 * This class handles the loading, validation, and storage of WGF files
 * from both core (built-in) and user (mod) directories.
 *
 * Example usage:
 * @code
 * WorldGenConfigLoader loader;
 * loader.SetLogCallback([](LogLevel level, const std::string& msg) {
 *     std::cout << "[WorldGen] " << msg << std::endl;
 * });
 *
 * if (loader.LoadFromDirectories("assets/worldgen/core",
 *                                "assets/worldgen/user")) {
 *     auto* frame = loader.GetWGFByUUID("some-uuid");
 *     if (frame) {
 *         // Use frame data
 *     }
 * }
 * @endcode
 */
class WorldGenConfigLoader {
 public:
    WorldGenConfigLoader() = default;
    ~WorldGenConfigLoader() = default;

    // Non-copyable, movable
    WorldGenConfigLoader(const WorldGenConfigLoader &) = delete;
    WorldGenConfigLoader &operator=(const WorldGenConfigLoader &) = delete;
    WorldGenConfigLoader(WorldGenConfigLoader &&) = default;
    WorldGenConfigLoader &operator=(WorldGenConfigLoader &&) = default;

    /**
     * @brief Sets the logging callback for status messages.
     *
     * @param callback Function to receive log messages.
     */
    void SetLogCallback(LogCallback callback);

    /**
     * @brief Loads WGF files from core and user directories.
     *
     * Core files are loaded first, then user files. If a user file has
     * a UUID that conflicts with a core file, the user file is skipped.
     *
     * @param core_path Path to core WGF directory.
     * @param user_path Path to user WGF directory.
     * @return True if at least one valid WGF was loaded.
     */
    bool LoadFromDirectories(
        const std::string &core_path, const std::string &user_path);

    /**
     * @brief Loads the global worldgen configuration file.
     *
     * @param config_path Path to config.json.
     * @return True if config was loaded successfully.
     */
    bool LoadGlobalConfig(const std::string &config_path);

    /**
     * @brief Retrieves a WGF definition by UUID.
     *
     * @param uuid The UUID to look up.
     * @return Pointer to the WGF definition, or nullptr if not found.
     */
    const WGFDefinition *GetWGFByUUID(const std::string &uuid) const;

    /**
     * @brief Returns all loaded WGF definitions.
     *
     * @return Const reference to the vector of all WGFs.
     */
    const std::vector<WGFDefinition> &GetAllWGFs() const;

    /**
     * @brief Returns the global configuration.
     *
     * @return Const reference to the WorldGenConfig.
     */
    const WorldGenConfig &GetConfig() const;

    /**
     * @brief Returns an ordered list of all loaded UUIDs.
     *
     * This list is deterministic (sorted) for seed generation.
     *
     * @return Vector of UUID strings.
     */
    std::vector<std::string> GetUUIDList() const;

    /**
     * @brief Returns loading statistics.
     *
     * @return Const reference to load statistics.
     */
    const LoadStatistics &GetStatistics() const;

    /**
     * @brief Checks if any WGFs were loaded successfully.
     *
     * @return True if at least one WGF is available.
     */
    bool HasWGFs() const;

    /**
     * @brief Finds WGFs matching the given tags.
     *
     * @param tags Tags to filter by.
     * @param match_all If true, WGF must have all tags; otherwise any tag.
     * @return Vector of pointers to matching WGFs.
     */
    std::vector<const WGFDefinition *> FindByTags(
        const std::vector<std::string> &tags, bool match_all = false) const;

    /**
     * @brief Finds WGFs within a difficulty range.
     *
     * @param min_difficulty Minimum difficulty (inclusive).
     * @param max_difficulty Maximum difficulty (inclusive).
     * @return Vector of pointers to matching WGFs.
     */
    std::vector<const WGFDefinition *> FindByDifficulty(
        float min_difficulty, float max_difficulty) const;

 private:
    /**
     * @brief Scans a directory for .wgf.json files.
     *
     * @param path Directory path to scan.
     * @return Vector of file paths found.
     */
    std::vector<std::filesystem::path> ScanDirectory(const std::string &path);

    /**
     * @brief Loads a single WGF file.
     *
     * @param filepath Path to the .wgf.json file.
     * @param is_core True if from core directory.
     * @return Parsed WGFDefinition, or nullopt on failure.
     */
    std::optional<WGFDefinition> LoadWGFFile(
        const std::filesystem::path &filepath, bool is_core);

    /**
     * @brief Validates required fields in a WGF definition.
     *
     * @param wgf The WGF to validate.
     * @return True if valid.
     */
    bool ValidateWGF(const WGFDefinition &wgf);

    /**
     * @brief Assigns default values to optional fields.
     *
     * @param wgf The WGF to update.
     */
    void AssignDefaults(WGFDefinition &wgf);

    /**
     * @brief Validates UUID format (UUIDv4).
     *
     * @param uuid The UUID string to validate.
     * @return True if valid UUIDv4 format.
     */
    bool ValidateUUID(const std::string &uuid);

    /**
     * @brief Logs a message using the configured callback.
     *
     * @param level Log level.
     * @param message Message to log.
     */
    void Log(LogLevel level, const std::string &message);

    std::vector<WGFDefinition> wgf_definitions_;
    std::unordered_map<std::string, size_t> uuid_to_index_;
    WorldGenConfig config_;
    LoadStatistics statistics_;
    LogCallback log_callback_;
};

}  // namespace worldgen
