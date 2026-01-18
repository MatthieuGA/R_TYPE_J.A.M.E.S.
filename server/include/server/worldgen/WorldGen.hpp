/**
 * @file WorldGen.hpp
 * @brief Main include file for the WorldGen system.
 *
 * Include this header to get access to all WorldGen functionality:
 * - WorldGenTypes: Data structures (WGFDefinition, ObstacleData, SpawnEvent,
 * etc.)
 * - WorldGenConfigLoader: Loading and validating WGF files from directories
 * - WorldGenManager: Runtime manager for deterministic world generation
 * - DeterministicRNG: PCG-based random number generator for reproducibility
 */
#pragma once

#include "server/worldgen/DeterministicRNG.hpp"
#include "server/worldgen/WorldGenConfigLoader.hpp"
#include "server/worldgen/WorldGenManager.hpp"
#include "server/worldgen/WorldGenTypes.hpp"
