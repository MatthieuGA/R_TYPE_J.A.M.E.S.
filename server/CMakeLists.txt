add_executable(r-type_server)
file(GLOB_RECURSE SERVER_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

target_sources(r-type_server PRIVATE ${SERVER_SOURCES})

# Find nlohmann_json package
find_package(nlohmann_json CONFIG REQUIRED)

# Include directories
target_include_directories(r-type_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/engine/include
    ${Boost_INCLUDE_DIRS}
)

# Link libraries (Boost components, threading support, and JSON)
target_link_libraries(r-type_server PRIVATE 
    r-type_engine
    Boost::system
    Threads::Threads
    nlohmann_json::nlohmann_json
)

# Platform-specific linking for networking
if(WIN32)
    # Windows requires ws2_32 for Boost.Asio
    target_link_libraries(r-type_server PRIVATE ws2_32 wsock32)
endif()
# Post build copy data folder
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    $<TARGET_FILE_DIR:r-type_server>/data
)

# Copy worldgen assets to build directory
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/worldgen
        $<TARGET_FILE_DIR:r-type_server>/assets/worldgen
    COMMENT "Copying worldgen assets to build directory"
)