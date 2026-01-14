/**
 * @file manual_test_worldgen.cpp
 * @brief Manual testing program for WorldGen system
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "server/worldgen/WorldGen.hpp"

using worldgen::DeterministicRNG;
using worldgen::LogLevel;
using worldgen::WorldGenConfigLoader;
using worldgen::WorldGenManager;

// ANSI color codes for better output
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

void PrintHeader(const std::string &title) {
    std::cout << "\n"
              << BOLD << CYAN
              << "========================================" << RESET << "\n";
    std::cout << BOLD << CYAN << " " << title << RESET << "\n";
    std::cout << BOLD << CYAN
              << "========================================" << RESET << "\n\n";
}

void PrintSuccess(const std::string &msg) {
    std::cout << GREEN << "✓ " << msg << RESET << "\n";
}

void PrintError(const std::string &msg) {
    std::cout << RED << "✗ " << msg << RESET << "\n";
}

void PrintInfo(const std::string &msg) {
    std::cout << BLUE << "ℹ " << msg << RESET << "\n";
}

void PrintWarning(const std::string &msg) {
    std::cout << YELLOW << "⚠ " << msg << RESET << "\n";
}

// Test 1: Load WGF files
void Test1_LoadWGFs() {
    PrintHeader("TEST 1: Load WGF Files");

    WorldGenConfigLoader loader;

    // Set up logging
    loader.SetLogCallback([](LogLevel level, const std::string &msg) {
        switch (level) {
            case LogLevel::kInfo:
                std::cout << BLUE << "[INFO] " << RESET << msg << "\n";
                break;
            case LogLevel::kWarning:
                std::cout << YELLOW << "[WARN] " << RESET << msg << "\n";
                break;
            case LogLevel::kError:
                std::cout << RED << "[ERROR] " << RESET << msg << "\n";
                break;
        }
    });

    bool success = loader.LoadFromDirectories(
        "assets/worldgen/core", "assets/worldgen/user");

    if (success) {
        PrintSuccess("WGF files loaded successfully");

        auto stats = loader.GetStatistics();
        std::cout << "  Files scanned: " << stats.total_files_scanned << "\n";
        std::cout << "  Core files loaded: " << stats.core_files_loaded
                  << "\n";
        std::cout << "  User files loaded: " << stats.user_files_loaded
                  << "\n";
        std::cout << "  Parse errors: " << stats.parse_errors << "\n";

        const auto &wgfs = loader.GetAllWGFs();
        std::cout << "\n" << BOLD << "Available WGFs:" << RESET << "\n";
        for (const auto &wgf : wgfs) {
            std::cout << "  • " << wgf.name
                      << " (difficulty: " << wgf.difficulty << ")\n";
            std::cout << "    UUID: " << CYAN << wgf.uuid << RESET << "\n";
            std::cout << "    Obstacles: " << wgf.obstacles.size() << "\n";
        }
    } else {
        PrintError("Failed to load WGF files");
    }
}

// Test 2: Deterministic RNG
void Test2_DeterministicRNG() {
    PrintHeader("TEST 2: Deterministic RNG");

    uint64_t seed = 12345;

    DeterministicRNG rng1(seed);
    DeterministicRNG rng2(seed);

    PrintInfo("Testing that same seed produces same sequence...");

    bool all_match = true;
    for (int i = 0; i < 10; ++i) {
        uint32_t val1 = rng1.Next();
        uint32_t val2 = rng2.Next();

        std::cout << "  Iteration " << i << ": " << val1 << " vs " << val2;

        if (val1 == val2) {
            std::cout << " " << GREEN << "✓" << RESET << "\n";
        } else {
            std::cout << " " << RED << "✗" << RESET << "\n";
            all_match = false;
        }
    }

    if (all_match) {
        PrintSuccess("Determinism verified!");
    } else {
        PrintError("Determinism failed!");
    }

    // Test other RNG functions
    PrintInfo("\nTesting other RNG functions:");
    DeterministicRNG rng3(seed);

    std::cout << "  NextInt(1, 100): " << rng3.NextInt(1, 100) << "\n";
    std::cout << "  NextFloatRange(0.0, 1.0): "
              << rng3.NextFloatRange(0.0f, 1.0f) << "\n";
    std::cout << "  NextBool(0.7): "
              << (rng3.NextBool(0.7f) ? "true" : "false") << "\n";

    std::vector<float> weights = {1.0f, 2.0f, 3.0f, 4.0f};
    std::cout << "  SelectWeighted({1,2,3,4}): "
              << rng3.SelectWeighted(weights) << "\n";
}

// Test 3: Endless Mode with Seed
void Test3_EndlessMode(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 3: Endless Mode with Seed");

    WorldGenManager manager(loader);

    uint64_t seed = 42;
    float difficulty = 5.0f;

    PrintInfo("Initializing endless mode with seed " + std::to_string(seed) +
              " and difficulty " + std::to_string(difficulty));

    bool success = manager.InitializeEndless(seed, difficulty);

    if (!success) {
        PrintError("Failed to initialize endless mode");
        return;
    }

    PrintSuccess("Endless mode initialized");

    const auto &metadata = manager.GetSeedMetadata();
    std::cout << "  Seed: " << metadata.seed_value << "\n";
    std::cout << "  Difficulty: " << metadata.target_difficulty << "\n";
    std::cout << "  Endless: " << (metadata.is_endless ? "yes" : "no") << "\n";
    std::cout << "  Allowed WGFs: " << metadata.allowed_wgf_uuids.size()
              << "\n";

    PrintInfo("\nGenerating first 5 frames:");
    for (int i = 0; i < 5; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            std::cout << "  Frame " << i << ": " << YELLOW << wgf->name
                      << RESET << " (difficulty: " << wgf->difficulty << ")\n";
            std::cout << "    Width: " << wgf->width << " units\n";
            std::cout << "    Obstacles: " << wgf->obstacles.size() << "\n";
        }
        manager.AdvanceFrame();
    }
}

// Test 4: Determinism Across Managers
void Test4_Determinism(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 4: Determinism Across Managers");

    uint64_t seed = 99999;

    WorldGenManager manager1(loader);
    WorldGenManager manager2(loader);

    PrintInfo("Creating two managers with same seed: " + std::to_string(seed));

    manager1.InitializeEndless(seed, 5.0f);
    manager2.InitializeEndless(seed, 5.0f);

    PrintInfo("Verifying they generate the same sequence:");

    bool all_match = true;
    for (int i = 0; i < 20; ++i) {
        const auto *wgf1 = manager1.GetCurrentWGF();
        const auto *wgf2 = manager2.GetCurrentWGF();

        if (wgf1 && wgf2) {
            bool match = (wgf1->uuid == wgf2->uuid);
            std::cout << "  Frame " << std::setw(2) << i << ": ";

            if (match) {
                std::cout << GREEN << "✓ " << RESET << wgf1->name << "\n";
            } else {
                std::cout << RED << "✗ " << RESET << wgf1->name << " vs "
                          << wgf2->name << "\n";
                all_match = false;
            }
        }

        manager1.AdvanceFrame();
        manager2.AdvanceFrame();
    }

    if (all_match) {
        PrintSuccess("Perfect determinism over 20 frames!");
    } else {
        PrintError("Determinism failed!");
    }
}

// Test 5: Reset Functionality
void Test5_Reset(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 5: Reset Functionality");

    WorldGenManager manager(loader);
    manager.InitializeEndless(7777, 4.0f);

    PrintInfo("Collecting first sequence:");
    std::vector<std::string> first_sequence;
    for (int i = 0; i < 10; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            first_sequence.push_back(wgf->uuid);
            std::cout << "  Frame " << i << ": " << wgf->name << "\n";
        }
        manager.AdvanceFrame();
    }

    PrintInfo("\nResetting manager...");
    manager.Reset();

    PrintInfo("Collecting second sequence:");
    std::vector<std::string> second_sequence;
    for (int i = 0; i < 10; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            second_sequence.push_back(wgf->uuid);
            std::cout << "  Frame " << i << ": " << wgf->name << "\n";
        }
        manager.AdvanceFrame();
    }

    if (first_sequence == second_sequence) {
        PrintSuccess("Reset works! Sequences match perfectly.");
    } else {
        PrintError("Reset failed! Sequences differ.");
    }
}

// Test 6: Save and Restore State
void Test6_SaveRestore(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 6: Save and Restore State");

    WorldGenManager manager(loader);
    manager.InitializeEndless(5555, 6.0f);

    PrintInfo("Advancing 5 frames...");
    for (int i = 0; i < 5; ++i) {
        manager.AdvanceFrame();
    }

    PrintInfo("Saving state...");
    worldgen::WorldGenState saved = manager.SaveState();
    std::cout << "  Frame index: " << saved.current_frame_index << "\n";
    std::cout << "  Difficulty: " << saved.current_difficulty << "\n";
    std::cout << "  Current WGF: " << saved.current_wgf_uuid << "\n";

    PrintInfo("Collecting next 5 frames after save:");
    std::vector<std::string> after_save;
    for (int i = 0; i < 5; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            after_save.push_back(wgf->uuid);
            std::cout << "  Frame " << i << ": " << wgf->name << "\n";
        }
        manager.AdvanceFrame();
    }

    PrintInfo("Restoring to saved state...");
    manager.RestoreState(saved);

    PrintInfo("Collecting frames after restore:");
    std::vector<std::string> after_restore;
    for (int i = 0; i < 5; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            after_restore.push_back(wgf->uuid);
            std::cout << "  Frame " << i << ": " << wgf->name << "\n";
        }
        manager.AdvanceFrame();
    }

    if (after_save == after_restore) {
        PrintSuccess("Save/Restore works! Sequences match.");
    } else {
        PrintError("Save/Restore failed! Sequences differ.");
    }
}

// Test 7: Level Mode
void Test7_LevelMode(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 7: Fixed Level Mode");

    WorldGenManager manager(loader);

    // Try to load a level file
    PrintInfo("Loading level from file...");
    bool loaded = manager.LoadLevelFromFile(
        "assets/worldgen/levels/tutorial.level.json");

    if (!loaded) {
        PrintWarning(
            "Could not load tutorial.level.json, creating one "
            "programmatically");

        // Create a level programmatically
        worldgen::LevelDefinition level;
        level.uuid = "test-level-001";
        level.name = "Test Level";
        level.author = "Manual Test";
        level.description = "A test level with predefined frames";

        // Get some WGF UUIDs
        const auto &wgfs = loader.GetAllWGFs();
        if (wgfs.size() >= 3) {
            level.frames.push_back(wgfs[0].uuid);
            level.frames.push_back(wgfs[1].uuid);
            level.frames.push_back(wgfs[0].uuid);
        } else {
            PrintError("Not enough WGFs loaded to create test level");
            return;
        }

        level.is_endless = false;
        level.target_difficulty = 5.0f;

        manager.AddLevel(level);

        PrintInfo("Initializing programmatic level...");
        if (!manager.InitializeLevel(level.uuid)) {
            PrintError("Failed to initialize level");
            return;
        }
    } else {
        // Get the loaded level
        const worldgen::LevelDefinition *level =
            manager.GetLevelByUUID("tutorial-0000-0000-0000-000000000001");

        if (!level) {
            PrintError("Could not find loaded level");
            return;
        }

        std::cout << "  Level: " << level->name << "\n";
        std::cout << "  Author: " << level->author << "\n";
        std::cout << "  Frames: " << level->frames.size() << "\n";

        PrintInfo("Initializing level...");
        if (!manager.InitializeLevel(level->uuid)) {
            PrintError("Failed to initialize level");
            return;
        }
    }

    PrintSuccess("Level initialized");

    PrintInfo("Playing through level:");
    int frame_count = 0;
    while (!manager.IsLevelComplete() && frame_count < 20) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            std::cout << "  Frame " << frame_count << ": " << wgf->name
                      << " (difficulty: " << wgf->difficulty << ")\n";
        }
        manager.AdvanceFrame();
        frame_count++;
    }

    if (manager.IsLevelComplete()) {
        PrintSuccess("Level completed!");
    } else {
        PrintWarning(
            "Level not complete after 20 frames (might be endless or very "
            "long)");
    }
}

// Test 8: Spawn Events
void Test8_SpawnEvents(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 8: Spawn Events");

    WorldGenManager manager(loader);

    int event_count = 0;
    int obstacle_count = 0;
    int frame_start_count = 0;

    // Set up callback
    manager.SetSpawnCallback([&](const worldgen::SpawnEvent &event) {
        event_count++;

        switch (event.type) {
            case worldgen::SpawnEvent::EventType::kObstacle:
                obstacle_count++;
                break;
            case worldgen::SpawnEvent::EventType::kFrameStart:
                frame_start_count++;
                std::cout << MAGENTA << "  [FRAME START] " << RESET
                          << "Frame #" << event.frame_number
                          << " at x=" << event.world_x << "\n";
                break;
            case worldgen::SpawnEvent::EventType::kFrameEnd:
                std::cout << CYAN << "  [FRAME END] " << RESET << "Frame #"
                          << event.frame_number << " at x=" << event.world_x
                          << "\n";
                break;
            default:
                break;
        }
    });

    manager.InitializeEndless(1111, 5.0f);

    PrintInfo("Advancing 3 frames and monitoring events:");
    for (int i = 0; i < 3; ++i) {
        const auto *wgf = manager.GetCurrentWGF();
        if (wgf) {
            std::cout << "\n"
                      << BOLD << "Frame " << i << ": " << wgf->name << RESET
                      << "\n";
        }
        manager.AdvanceFrame();
    }

    std::cout << "\n" << BOLD << "Event Summary:" << RESET << "\n";
    std::cout << "  Total events: " << event_count << "\n";
    std::cout << "  Frame starts: " << frame_start_count << "\n";
    std::cout << "  Obstacles: " << obstacle_count << "\n";

    if (event_count > 0) {
        PrintSuccess("Spawn events are being generated!");
    } else {
        PrintWarning("No events generated (this is unusual)");
    }
}

// Test 9: Difficulty Scaling
void Test9_DifficultyScaling(WorldGenConfigLoader &loader) {
    PrintHeader("TEST 9: Difficulty Scaling");

    PrintInfo("Testing frame selection at different difficulty levels:");

    float difficulties[] = {1.0f, 3.0f, 5.0f, 7.0f, 9.0f};

    for (float diff : difficulties) {
        std::cout << "\n"
                  << BOLD << "Difficulty " << diff << ":" << RESET << "\n";

        WorldGenManager manager(loader);
        manager.InitializeEndless(8888, diff);

        float total_difficulty = 0.0f;
        int count = 0;

        for (int i = 0; i < 10; ++i) {
            const auto *wgf = manager.GetCurrentWGF();
            if (wgf) {
                total_difficulty += wgf->difficulty;
                count++;

                // Show first 3
                if (i < 3) {
                    std::cout << "  • " << wgf->name
                              << " (difficulty: " << wgf->difficulty << ")\n";
                }
            }
            manager.AdvanceFrame();
        }

        float avg = count > 0 ? total_difficulty / count : 0.0f;
        std::cout << "  Average difficulty: " << avg << " (target: " << diff
                  << ")\n";
    }
}

// Interactive Menu
void InteractiveMenu(WorldGenConfigLoader &loader) {
    PrintHeader("INTERACTIVE WORLDGEN TESTER");

    while (true) {
        std::cout << "\n" << BOLD << "Select a test:" << RESET << "\n";
        std::cout << "  1. Load WGF Files\n";
        std::cout << "  2. Test Deterministic RNG\n";
        std::cout << "  3. Test Endless Mode\n";
        std::cout << "  4. Test Determinism Across Managers\n";
        std::cout << "  5. Test Reset Functionality\n";
        std::cout << "  6. Test Save/Restore State\n";
        std::cout << "  7. Test Fixed Level Mode\n";
        std::cout << "  8. Test Spawn Events\n";
        std::cout << "  9. Test Difficulty Scaling\n";
        std::cout << "  0. Run All Tests\n";
        std::cout << "  q. Quit\n";
        std::cout << "\nChoice: ";

        std::string input;
        std::getline(std::cin, input);

        if (input == "q" || input == "Q") {
            break;
        }

        try {
            int choice = std::stoi(input);

            switch (choice) {
                case 1:
                    Test1_LoadWGFs();
                    break;
                case 2:
                    Test2_DeterministicRNG();
                    break;
                case 3:
                    Test3_EndlessMode(loader);
                    break;
                case 4:
                    Test4_Determinism(loader);
                    break;
                case 5:
                    Test5_Reset(loader);
                    break;
                case 6:
                    Test6_SaveRestore(loader);
                    break;
                case 7:
                    Test7_LevelMode(loader);
                    break;
                case 8:
                    Test8_SpawnEvents(loader);
                    break;
                case 9:
                    Test9_DifficultyScaling(loader);
                    break;
                case 0:
                    Test1_LoadWGFs();
                    Test2_DeterministicRNG();
                    Test3_EndlessMode(loader);
                    Test4_Determinism(loader);
                    Test5_Reset(loader);
                    Test6_SaveRestore(loader);
                    Test7_LevelMode(loader);
                    Test8_SpawnEvents(loader);
                    Test9_DifficultyScaling(loader);
                    break;
                default:
                    PrintError("Invalid choice");
            }
        } catch (...) {
            PrintError("Invalid input");
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
}

int main() {
    std::cout << BOLD << GREEN << R"(
╦ ╦┌─┐┬─┐┬  ┌┬┐╔═╗┌─┐┌┐┌  ╔╦╗┌─┐┌┐┌┬ ┬┌─┐┬  
║║║│ │├┬┘│   ││║ ╦├┤ │││  ║║║├─┤││││ │├─┤│  
╚╩╝└─┘┴└─┴─┘─┴┘╚═╝└─┘┘└┘  ╩ ╩┴ ┴┘└┘└─┘┴ ┴┴─┘
╔╦╗┌─┐┌─┐┌┬┐┌─┐┬─┐                          
 ║ ├┤ └─┐ │ ├┤ ├┬┘                          
 ╩ └─┘└─┘ ┴ └─┘┴└─                          
    )" << RESET
              << "\n";

    PrintInfo("Initializing WorldGen ConfigLoader...");

    WorldGenConfigLoader loader;
    loader.SetLogCallback([](LogLevel level, const std::string &msg) {
        // Silent for menu
    });

    bool loaded = loader.LoadFromDirectories(
        "assets/worldgen/core", "assets/worldgen/user");

    if (!loaded || !loader.HasWGFs()) {
        PrintError("Failed to load WGF files!");
        PrintInfo("Make sure you're running from the server build directory");
        PrintInfo("Expected: build/server/");
        return 1;
    }

    PrintSuccess(
        "Loaded " + std::to_string(loader.GetAllWGFs().size()) + " WGF files");

    InteractiveMenu(loader);

    std::cout << "\n"
              << BOLD << GREEN << "Thanks for testing WorldGen! 🚀" << RESET
              << "\n\n";

    return 0;
}
