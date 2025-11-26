# Enable Google Test
include(GoogleTest)

# Create test executable
add_executable(engine_tests)


# Create test executable
file(GLOB_RECURSE R_TYPE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
target_sources(engine_tests PRIVATE ${R_TYPE_CLIENT_SOURCES})

# Link with Google Test and the engine library
target_link_libraries(engine_tests
    PRIVATE
        GTest::gtest_main
        r-type_engine
)
target_link_libraries(engine_tests PRIVATE r-type_engine
    SFML::Window SFML::Graphics SFML::Network SFML::Audio SFML::System)

# Discover tests for CTest
gtest_discover_tests(engine_tests)
