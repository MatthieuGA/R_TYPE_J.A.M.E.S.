# Enable Google Test
include(GoogleTest)

# Create test executable
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
