add_executable(r-type_client)

file(GLOB_RECURSE R_TYPE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Find threading library for portability
find_package(Threads REQUIRED)

target_sources(r-type_client PRIVATE ${R_TYPE_CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(r-type_client PRIVATE r-type_engine
    SFML::Window SFML::Graphics SFML::Network SFML::Audio SFML::System
    Boost::system Threads::Threads)

# Post build copy assets
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    $<TARGET_FILE_DIR:r-type_client>/assets
)