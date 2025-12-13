cmake_minimum_required(VERSION 3.23)

project(RTypeProject LANGUAGES CXX)

# ===========================
# Debug output options
# ===========================
option(DEBUG_PARTICLES "Enable particle rendering debug output" OFF)
option(DEBUG_RENDERING "Enable general rendering debug output" OFF)
option(DEBUG_NETWORK "Enable network debug output" OFF)

# Pass debug flags to compiler
if(DEBUG_PARTICLES)
    add_compile_definitions(DEBUG_PARTICLES)
    message(STATUS "Particle debug output: ENABLED")
endif()

if(DEBUG_RENDERING)
    add_compile_definitions(DEBUG_RENDERING)
    message(STATUS "Rendering debug output: ENABLED")
endif()

if(DEBUG_NETWORK)
    add_compile_definitions(DEBUG_NETWORK)
    message(STATUS "Network debug output: ENABLED")
endif()

# Enable all debug in Debug build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(CMAKE_BUILD_TYPE_DEBUG)
    message(STATUS "Debug build: All debug outputs ENABLED")
endif()

# ===========================
# Options de coverage
# ===========================
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Si on n'est PAS sur MSVC et que le coverage est activé,
# on ajoute les flags GCC/Clang (--coverage).
if (ENABLE_COVERAGE AND NOT MSVC)
    message(STATUS "Coverage flags enabled for GCC/Clang")
    add_compile_options(-g -O0 --coverage)
    add_link_options(--coverage)
endif()

# Find required packages (vcpkg integration)
# SFML 2.6.2 uses lowercase component names
find_package(SFML COMPONENTS system window graphics network audio CONFIG REQUIRED)

# Boost components for networking and lock-free data structures
# Use CONFIG mode with vcpkg (CMP0167 compliance)
find_package(Boost REQUIRED COMPONENTS system CONFIG)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Explanation: vcpkg's OpenAL-soft has a missing dependency on fmt
# Add fmt as an INTERFACE dependency to SFML::Audio so it's linked in correct order
if (TARGET SFML::Audio)
    target_link_libraries(SFML::Audio INTERFACE fmt::fmt)
elseif (TARGET sfml-audio)
    target_link_libraries(sfml-audio INTERFACE fmt::fmt)
endif()

# Provide compatibility aliases so code using SFML::Name targets works with
# vcpkg-provided SFML 2.x (which exports sfml-name targets).
if (NOT TARGET SFML::Window)
    if (TARGET sfml-window)
        add_library(SFML::Window ALIAS sfml-window)
    endif()
    if (TARGET sfml-graphics)
        add_library(SFML::Graphics ALIAS sfml-graphics)
    endif()
    if (TARGET sfml-network)
        add_library(SFML::Network ALIAS sfml-network)
    endif()
    if (TARGET sfml-audio)
        add_library(SFML::Audio ALIAS sfml-audio)
    endif()
    if (TARGET sfml-system)
        add_library(SFML::System ALIAS sfml-system)
    endif()
endif()

# GoogleTest for unit testing
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ===========================
# Sous-répertoires
# ===========================
add_subdirectory(engine)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tests)

# ===========================
# Target run-coverage
# ===========================
# Sur Windows / MSVC : utiliser OpenCppCoverage
if (MSVC)
    add_custom_target(run-coverage
        COMMAND OpenCppCoverage
            --cover_children
            --export_type html:${CMAKE_BINARY_DIR}/coverage.html
            -- ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests with OpenCppCoverage and generating coverage.html"
    )
else()
    # Sur GCC/Clang : utiliser gcovr si disponible
    find_program(GCOVR_PATH gcovr)
    if (GCOVR_PATH)
        add_custom_target(run-coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_PATH}
                --root ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/engine
                --filter ${CMAKE_SOURCE_DIR}/client
                --exclude ${CMAKE_SOURCE_DIR}/tests
                --html ${CMAKE_BINARY_DIR}/coverage.html
                --html-details
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating coverage.html with gcovr"
        )
    else()
        message(STATUS "gcovr not found: run-coverage target (non-MSVC) will not be available")
    endif()
endif()

# Installation rules
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION bin/assets/ OPTIONAL)

# Print configuration summary
message(STATUS "==============================================")
message(STATUS "R-TYPE J.A.M.E.S. Configuration Summary")
message(STATUS "==============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "SFML Found: ${SFML_FOUND}")
message(STATUS "Boost Found: ${Boost_FOUND}")
if(Boost_FOUND)
    message(STATUS "  Boost Version: ${Boost_VERSION}")
    message(STATUS "  Boost Include: ${Boost_INCLUDE_DIRS}")
endif()
message(STATUS "Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "==============================================")
