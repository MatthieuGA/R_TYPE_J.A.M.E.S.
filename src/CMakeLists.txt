# Source files for the client
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET r-type_client)

set(SRC
    ${SRC_ROOT}/main.cpp
)

# Create the client executable
add_executable(${TARGET} ${SRC})

# Set output directory
set_target_properties(${TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/
)

# Link libraries
target_link_libraries(${TARGET}
    PRIVATE SFML::Window
    PRIVATE SFML::Graphics
    PRIVATE SFML::Network
    PRIVATE SFML::Audio
    PRIVATE Boost::asio
    PRIVATE Boost::system
)

# Include directories
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler warnings
if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4)
else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()
