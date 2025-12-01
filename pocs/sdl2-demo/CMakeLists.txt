cmake_minimum_required(VERSION 3.20)
project(r-type-poc-sdl2 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# Create executable
add_executable(r-type_sdl2
    src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/src/entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/src/registry.cpp
)

target_link_libraries(r-type_sdl2 PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

target_include_directories(r-type_sdl2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Copy shared assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
