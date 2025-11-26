cmake_minimum_required(VERSION 3.20)
project(SFML_PoC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Add ECS library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ecs ${CMAKE_CURRENT_BINARY_DIR}/ecs)

# Create executable
add_executable(sfml_poc
    src/main.cpp
)

target_link_libraries(sfml_poc PRIVATE
    poc_ecs
    sfml-graphics
    sfml-window
    sfml-system
)

# Copy shared assets from project root
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
