# Source files for the client
set(CLIENT_SOURCES
    main.cpp
)

# Create the client executable
add_executable(r-type_client ${CLIENT_SOURCES})

# Link libraries
target_link_libraries(r-type_client PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    SFML::Network
    SFML::Audio
    asio::asio
)

# Include directories
target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler warnings
if(MSVC)
    target_compile_options(r-type_client PRIVATE /W4)
else()
    target_compile_options(r-type_client PRIVATE -Wall -Wextra -Wpedantic)
endif()
