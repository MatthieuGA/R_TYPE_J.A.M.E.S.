cmake_minimum_required(VERSION 3.20)
project(SFML_PoC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Create executable
add_executable(sfml_poc
    src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/src/entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/src/registry.cpp
)

target_include_directories(sfml_poc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sfml_poc PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Copy shared assets from project root
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
