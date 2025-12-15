# Enable Google Test
include(GoogleTest)

# Gather test sources and create test executable
file(GLOB_RECURSE R_TYPE_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Exclude server-specific tests (compiled separately with server sources/includes)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packets.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_client_connection_manager.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packet_handler.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packet_sender.cpp")

# Exclude command-line parser tests (compiled separately without network dependencies)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_command_line_parser.cpp")

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
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/client/network
    ${CMAKE_SOURCE_DIR}/external
)

# Compile client engine sources into the test executable so systems are available
file(GLOB_RECURSE CLIENT_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/client/engine/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/game/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/include/*.cpp"
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
# TODO: Fix test_packet_handler.cpp and test_packet_sender.cpp for new API
# (PacketSender and PacketHandler constructors now require Network parameter)
add_executable(server_tests
    test_server.cpp
    test_packets.cpp
    test_config.cpp
    test_client_connection_manager.cpp
    # test_packet_handler.cpp  # TEMPORARILY DISABLED - needs API update
    # test_packet_sender.cpp    # TEMPORARILY DISABLED - needs API update
    ${SERVER_SOURCES}
)

# Include directories for server tests
target_include_directories(server_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/server/include
        ${CMAKE_SOURCE_DIR}/engine/include
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
