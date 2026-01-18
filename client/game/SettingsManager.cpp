/**
 * @file SettingsManager.cpp
 * @brief Implementation of non-template SettingsManager methods.
 */

#include "include/SettingsManager.hpp"
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <string>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

namespace Rtype::Client {

SettingsManager::SettingsManager(const std::string &file_path)
    : file_path_(file_path) {}

bool SettingsManager::EnsureDirectoryExists(const std::string &path) const {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        if (info.st_mode & S_IFDIR) {
            return true;  // Directory exists
        }
        std::cerr << "[SettingsManager] Path exists but is not a directory: "
                  << path << std::endl;
        return false;
    }

    // Try to create directory
    if (mkdir(path.c_str(), 0755) == 0) {
        std::cout << "[SettingsManager] Created directory: " << path
                  << std::endl;
        return true;
    }

    // Maybe parent doesn't exist - try recursive creation
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        std::string parent = path.substr(0, last_slash);
        if (EnsureDirectoryExists(parent)) {
            // Try again after creating parent
            if (mkdir(path.c_str(), 0755) == 0) {
                std::cout << "[SettingsManager] Created directory: " << path
                          << std::endl;
                return true;
            }
        }
    }

    std::cerr << "[SettingsManager] Failed to create directory: " << path
              << std::endl;
    return false;
}

}  // namespace Rtype::Client
