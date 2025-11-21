cmake_minimum_required(VERSION 3.23)

project(RTypeProject)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

add_subdirectory(engine)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tests)