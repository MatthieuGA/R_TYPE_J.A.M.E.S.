# Enable Google Test
include(GoogleTest)

# Gather test sources and create test executable
file(GLOB_RECURSE R_TYPE_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Exclude server-specific tests (compiled separately with server sources/includes)
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp")
list(REMOVE_ITEM R_TYPE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_packets.cpp")

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
)

# Compile client engine sources into the test executable so systems are available
file(GLOB_RECURSE CLIENT_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/client/engine/*.cpp"
    "${CMAKE_SOURCE_DIR}/client/game/*.cpp"
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

# Create test executable for server
add_executable(server_tests
    test_server.cpp
    test_packets.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Server.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Config.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Network.cpp
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
