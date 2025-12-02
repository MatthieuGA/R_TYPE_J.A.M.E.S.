add_executable(r-type_client)

file(GLOB_RECURSE R_TYPE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

target_sources(r-type_client PRIVATE ${R_TYPE_CLIENT_SOURCES})

# Include directories
target_include_directories(r-type_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(r-type_client PRIVATE 
    r-type_engine
    SFML::Window 
    SFML::Graphics 
    SFML::Network 
    SFML::Audio 
    SFML::System
    Boost::system
    Threads::Threads
)

# Platform-specific linking for networking
if(WIN32)
    # Windows requires ws2_32 for Boost.Asio
    target_link_libraries(r-type_client PRIVATE ws2_32 wsock32)
endif()

# Post build copy assets
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    $<TARGET_FILE_DIR:r-type_client>/Assets
)