# SFML Audio Plugin
cmake_minimum_required(VERSION 3.23)

# Define the plugin as a shared library
add_library(sfml_audio_module SHARED
    SFMLAudioModule.cpp
    SFMLAudioModule.hpp
)

# Link SFML Audio
target_link_libraries(sfml_audio_module
    PRIVATE
        SFML::Audio
)

# Include directories
target_include_directories(sfml_audio_module
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/client/plugins/audio/sfml
)

# Set C++ standard
target_compile_features(sfml_audio_module PRIVATE cxx_std_23)

# Remove "lib" prefix and set output directory
set_target_properties(sfml_audio_module PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Install the plugin
install(TARGETS sfml_audio_module
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)
