# SFML Video Plugin CMakeLists.txt
cmake_minimum_required(VERSION 3.23)

project(sfml_video_module)

# Find SFML
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Create shared library
add_library(sfml_video_module SHARED
    SFMLVideoModule.cpp
    SFMLVideoModule.hpp
)

# Include directories
target_include_directories(sfml_video_module PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/client/plugins/video/sfml
)

# Link SFML
target_link_libraries(sfml_video_module PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Set output properties
set_target_properties(sfml_video_module PROPERTIES
    PREFIX ""  # No 'lib' prefix
    OUTPUT_NAME "sfml_video_module"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# C++23 standard
target_compile_features(sfml_video_module PRIVATE cxx_std_23)

# Position independent code
set_target_properties(sfml_video_module PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Install
install(TARGETS sfml_video_module
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)
