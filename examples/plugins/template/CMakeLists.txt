cmake_minimum_required(VERSION 3.23)
project(template_graphics_plugin)

# Ensure C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the shared library plugin
add_library(graphics_template SHARED
    src/template_plugin.cpp
)

# Include engine headers (for IRenderContext.hpp and plugin_api.h)
# Adjust the path based on where this template is extracted relative to the engine
target_include_directories(graphics_template PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../engine/include
)

# Platform-specific library naming
if(WIN32)
    # Windows: graphics_template.dll (no "lib" prefix)
    set_target_properties(graphics_template PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
else()
    # Linux/macOS: libgraphics_template.so / .dylib
    if(APPLE)
        set_target_properties(graphics_template PROPERTIES
            PREFIX "lib"
            SUFFIX ".dylib"
        )
    else()
        set_target_properties(graphics_template PROPERTIES
            PREFIX "lib"
            SUFFIX ".so"
        )
    endif()
endif()

# Compiler-specific warnings and settings
if(MSVC)
    target_compile_options(graphics_template PRIVATE /W4 /WX)
else()
    target_compile_options(graphics_template PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -fvisibility=hidden
    )
endif()

# Installation: Copy plugin to build/plugins directory for easy testing
if(WIN32)
    install(TARGETS graphics_template
        LIBRARY DESTINATION plugins
        RUNTIME DESTINATION plugins
    )
else()
    install(TARGETS graphics_template
        LIBRARY DESTINATION plugins
    )
endif()

# Convenience: Print build output location
add_custom_command(TARGET graphics_template POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Template plugin built successfully!"
    COMMAND ${CMAKE_COMMAND} -E echo "Location: $<TARGET_FILE:graphics_template>"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
)
