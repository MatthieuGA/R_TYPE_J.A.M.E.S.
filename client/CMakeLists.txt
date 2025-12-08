add_executable(r-type_client)

file(GLOB_RECURSE R_TYPE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Avoid compiling duplicate/refactored files present in both the root
# systems folder and the `Render/` subfolder which would cause
# multiple-definition link errors. Prefer the `Render/` implementations.
list(REMOVE_ITEM R_TYPE_CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Systems/SytemsFunctions/animationSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Systems/SytemsFunctions/drawableSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Systems/SytemsFunctions/initializeAnimationSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Systems/SytemsFunctions/initializeShaderSystem.cpp"
)

# Find threading library for portability
find_package(Threads REQUIRED)

target_sources(r-type_client PRIVATE ${R_TYPE_CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS})
target_link_libraries(r-type_client PRIVATE r-type_engine
    SFML::Window SFML::Graphics SFML::Network SFML::Audio SFML::System
    Boost::system Threads::Threads)

# Post build copy assets
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    $<TARGET_FILE_DIR:r-type_client>/Assets
)