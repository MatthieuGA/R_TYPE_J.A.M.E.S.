# Enable Google Test
include(GoogleTest)

# Find nlohmann_json package for worldgen tests
find_package(nlohmann_json CONFIG REQUIRED)

# Gather test sources and create test executable
file(GLOB_RECURSE R_TYPE_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Exclude server-specific tests (compiled separately with server sources/includes)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packets.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_worldgen.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_client_connection_manager.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packet_handler.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packet_sender.cpp")

# Exclude command-line parser tests (compiled separately without network dependencies)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_command_line_parser.cpp")

# Exclude plugin loader tests (compiled separately with plugin-specific dependencies)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_dl_loader.cpp")

# Exclude integration tests (compiled separately with test helpers)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/integration/smoke_plugin_load.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/integration/pixel_compare_test.cpp")

if (R_TYPE_TEST_SOURCES)
    add_executable(engine_tests ${R_TYPE_TEST_SOURCES})
else()
    message(FATAL_ERROR "No test source files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Link with Google Test, the engine library and SFML
target_link_libraries(engine_tests
    PRIVATE
        GTest::gtest_main
        r-type_engine
        SFML::Window SFML::Graphics SFML::Network SFML::Audio SFML::System
)

# Ensure test target can find client headers (client/include and client/Engine)
target_include_directories(engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/client/Engine
    ${CMAKE_SOURCE_DIR}/client/network
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/engine/include
)

# Compile client engine sources into the test executable so systems are available
file(GLOB_RECURSE CLIENT_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/client/engine/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/game/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/include/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/input/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/platform/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/graphics/*.cpp"
)

if (CLIENT_ENGINE_SOURCES)
    target_sources(engine_tests PRIVATE ${CLIENT_ENGINE_SOURCES})
endif()

# Add client Network implementation for network tests
target_sources(engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/client/network/Network.cpp
)

# Add Boost for network tests
target_link_libraries(engine_tests PRIVATE Boost::system)

# Discover tests for CTest
gtest_discover_tests(engine_tests)

# Gather server source files recursively
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/server/src/**/*.cpp"
)

# Create test executable for server
add_executable(server_tests
    test_server.cpp
    test_packets.cpp
    test_config.cpp
    test_client_connection_manager.cpp
    test_packet_handler.cpp
    test_packet_sender.cpp
    ${SERVER_SOURCES}
)

# Include directories for server tests
target_include_directories(server_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/server/include
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/external
)

# Link with Google Test, engine, and Boost
target_link_libraries(server_tests
    PRIVATE
        GTest::gtest_main
        r-type_engine
        Boost::system
)

# Discover server tests for CTest
gtest_discover_tests(server_tests)

# Create test executable for worldgen
add_executable(worldgen_tests
    test_worldgen.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/worldgen/WorldGenConfigLoader.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/worldgen/WorldGenManager.cpp
)

# Include directories for worldgen tests
target_include_directories(worldgen_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/server/include
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/external
)

# Link with Google Test and nlohmann_json
target_link_libraries(worldgen_tests
    PRIVATE
        GTest::gtest_main
        nlohmann_json::nlohmann_json
)

# Discover worldgen tests for CTest
gtest_discover_tests(worldgen_tests)

# Create separate test executable for command-line parser (no network dependencies)
# This avoids thread safety issues with death tests + Boost.Asio threads
add_executable(cli_parser_tests
    test_command_line_parser.cpp
    ${CMAKE_SOURCE_DIR}/client/game/CommandLineParser.cpp
)

# Include directories for CLI parser tests
target_include_directories(cli_parser_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/client/include
)

# Link with Google Test only (no network libraries)
target_link_libraries(cli_parser_tests
    PRIVATE
        GTest::gtest_main
)

# Discover CLI parser tests for CTest
gtest_discover_tests(cli_parser_tests)

# Create separate test executable for ServerSpawner
# Requires filesystem and threading support
add_executable(server_spawner_tests
    test_server_spawner.cpp
    ${CMAKE_SOURCE_DIR}/client/game/ServerSpawner.cpp
)

# Include directories for ServerSpawner tests
target_include_directories(server_spawner_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/client/include
)

# Link with Google Test and threading
find_package(Threads REQUIRED)
target_link_libraries(server_spawner_tests
    PRIVATE
        GTest::gtest_main
        Threads::Threads
)

# Discover ServerSpawner tests for CTest
gtest_discover_tests(server_spawner_tests)

# ========================================
# Plugin Loader Tests (Phase E)
# ========================================
# Create separate test executable for plugin loader tests
# These tests load actual shared libraries and verify error handling
add_executable(plugin_loader_tests
    unit/test_dl_loader.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsPluginLoader.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsBackendFactory.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/BackendResolver.cpp
)

# Include directories for plugin loader tests
target_include_directories(plugin_loader_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/client/include
        ${CMAKE_SOURCE_DIR}/client/graphics
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/external
)

# Link with Google Test and SFML
target_link_libraries(plugin_loader_tests
    PRIVATE
        GTest::gtest_main
        r-type_engine
        SFML::Graphics SFML::Window SFML::System
)

# Discover plugin loader tests for CTest
gtest_discover_tests(plugin_loader_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# ========================================
# Integration Tests (Phase E)
# ========================================
# Integration smoke test: verify plugin loads and renders
add_executable(integration_smoke_test
    integration/smoke_plugin_load.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsPluginLoader.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsBackendFactory.cpp
)

target_include_directories(integration_smoke_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/client/include
        ${CMAKE_SOURCE_DIR}/client/graphics
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
)

target_link_libraries(integration_smoke_test
    PRIVATE
        GTest::gtest_main
        r-type_engine
        SFML::Graphics SFML::Window SFML::System
)

gtest_discover_tests(integration_smoke_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Pixel comparison test: verify rendering output matches baseline
add_executable(pixel_compare_test
    integration/pixel_compare_test.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsPluginLoader.cpp
    ${CMAKE_SOURCE_DIR}/client/graphics/GraphicsBackendFactory.cpp
)

target_include_directories(pixel_compare_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/client/include
        ${CMAKE_SOURCE_DIR}/client/graphics
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
)

target_link_libraries(pixel_compare_test
    PRIVATE
        GTest::gtest_main
        r-type_engine
        SFML::Graphics SFML::Window SFML::System
)

gtest_discover_tests(pixel_compare_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
