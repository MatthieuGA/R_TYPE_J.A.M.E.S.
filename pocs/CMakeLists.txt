cmake_minimum_required(VERSION 3.20)
project(Graphics_PoCs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add ECS library (shared by all PoCs)
add_subdirectory(ecs)

# Add SFML PoC
option(BUILD_SFML_POC "Build SFML PoC" ON)
if(BUILD_SFML_POC)
    add_subdirectory(sfml-demo)
endif()

# Add SDL2 PoC
option(BUILD_SDL2_POC "Build SDL2 PoC" ON)
if(BUILD_SDL2_POC)
    add_subdirectory(sdl2-demo)
endif()

# Add Raylib PoC (when ready)
option(BUILD_RAYLIB_POC "Build Raylib PoC" OFF)
if(BUILD_RAYLIB_POC)
    # add_subdirectory(raylib-demo)
endif()
