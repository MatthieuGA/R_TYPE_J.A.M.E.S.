# Enable Google Test
include(GoogleTest)

# Gather test sources and create test executable
file(GLOB_RECURSE R_TYPE_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

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

# Discover tests for CTest
gtest_discover_tests(engine_tests)
