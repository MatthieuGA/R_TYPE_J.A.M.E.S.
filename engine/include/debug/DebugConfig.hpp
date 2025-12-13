/**
 * @file DebugConfig.hpp
 * @brief Compile-time debug configuration for the engine.
 *
 * Use CMake options to enable specific debug outputs:
 * - cmake -DDEBUG_PARTICLES=ON to enable particle rendering debug
 * - cmake -DDEBUG_RENDERING=ON to enable general rendering debug
 * - cmake -DDEBUG_NETWORK=ON to enable network debug
 * - cmake -DCMAKE_BUILD_TYPE=Debug to enable all debug features
 */

#pragma once

#include <iostream>

// Define debug macros based on CMake options
// These can be enabled individually or all at once in Debug builds

#ifdef CMAKE_BUILD_TYPE_DEBUG
#ifndef DEBUG_PARTICLES
#define DEBUG_PARTICLES
#endif
#ifndef DEBUG_RENDERING
#define DEBUG_RENDERING
#endif
#endif

// Particle rendering debug output
#ifdef DEBUG_PARTICLES
#define DEBUG_PARTICLES_LOG(msg) \
    std::cout << "[DEBUG_PARTICLES] " << msg << std::endl
#else
#define DEBUG_PARTICLES_LOG(msg) ((void)0)
#endif

// General rendering debug output
#ifdef DEBUG_RENDERING
#define DEBUG_RENDERING_LOG(msg) \
    std::cout << "[DEBUG_RENDERING] " << msg << std::endl
#else
#define DEBUG_RENDERING_LOG(msg) ((void)0)
#endif

// Network debug output
#ifdef DEBUG_NETWORK
#define DEBUG_NETWORK_LOG(msg) \
    std::cout << "[DEBUG_NETWORK] " << msg << std::endl
#else
#define DEBUG_NETWORK_LOG(msg) ((void)0)
#endif
