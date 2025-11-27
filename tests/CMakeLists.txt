# Enable Google Test
include(GoogleTest)

# Create test executable for engine
add_executable(engine_tests
    test_sparse_array.cpp
    test_registry.cpp
    test_zipper.cpp
)

# Link with Google Test and the engine library
target_link_libraries(engine_tests
    PRIVATE
        GTest::gtest_main
        r-type_engine
)

# Discover tests for CTest
gtest_discover_tests(engine_tests)

# Create test executable for server
add_executable(server_tests
    test_server.cpp
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

# Create manual test executable for interactive ECS testing
add_executable(server_manual_test
    server_manual_test.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Server.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Config.cpp
    ${CMAKE_SOURCE_DIR}/server/src/server/Network.cpp
)

target_include_directories(server_manual_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/server/include
        ${CMAKE_SOURCE_DIR}/engine/include
)

target_link_libraries(server_manual_test
    PRIVATE
        r-type_engine
        Boost::system
)
