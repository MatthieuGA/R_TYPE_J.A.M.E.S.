# Graphics SFML Plugin
# Builds as a standalone shared library that implements the graphics plugin ABI

# The parent project has already found these packages, but we'll ensure they're available
find_package(SFML 2.6 COMPONENTS graphics window system CONFIG REQUIRED)
# Link against parent's aliases if they exist
if(NOT TARGET SFML::graphics)
    if(TARGET sfml-graphics)
        add_library(SFML::graphics ALIAS sfml-graphics)
    endif()
endif()
if(NOT TARGET SFML::window)
    if(TARGET sfml-window)
        add_library(SFML::window ALIAS sfml-window)
    endif()
endif()
if(NOT TARGET SFML::system)
    if(TARGET sfml-system)
        add_library(SFML::system ALIAS sfml-system)
    endif()
endif()

# Create shared library
add_library(graphics_sfml SHARED)

target_sources(graphics_sfml PRIVATE
    sfml_plugin.cpp
)

# Include paths
target_include_directories(graphics_sfml PRIVATE
    # Engine headers for plugin_api.h and IRenderContext.hpp
    ${CMAKE_SOURCE_DIR}/engine/include
    # Client headers for SFMLRenderContext.hpp
    ${CMAKE_SOURCE_DIR}/client/graphics
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/client
)

# Link against SFML (using our aliases)
target_link_libraries(graphics_sfml PRIVATE
    SFML::graphics
    SFML::window
    SFML::system
)

# Compiler flags
target_compile_features(graphics_sfml PRIVATE cxx_std_20)

# Visibility: export symbols on all platforms
if(MSVC)
    # MSVC: use __declspec(dllexport) (handled in plugin_api.h)
else()
    # GCC/Clang: make all symbols visible by default for dlsym access
    target_compile_options(graphics_sfml PRIVATE -fvisibility=default)
endif()

# Output to a plugins directory next to the binary
set_target_properties(graphics_sfml PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Debug output
message(STATUS "[Plugin] Building graphics_sfml plugin")
message(STATUS "  Output: ${CMAKE_BINARY_DIR}/plugins")
