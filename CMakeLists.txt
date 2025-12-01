cmake_minimum_required(VERSION 3.23)

project(RTypeProject LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages (vcpkg integration)
find_package(SFML COMPONENTS Window Graphics Network Audio CONFIG REQUIRED)
# Use Boost.Asio (provided via vcpkg `boost-asio`)
find_package(Boost REQUIRED COMPONENTS asio system)

# GoogleTest for unit testing
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tests)

# Installation rules
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION bin/assets/ OPTIONAL)

# Print configuration summary
message(STATUS "==============================================")
message(STATUS "R-TYPE J.A.M.E.S. Configuration Summary")
message(STATUS "==============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "SFML Found: ${SFML_FOUND}")
message(STATUS "Boost Found: ${Boost_FOUND}")
message(STATUS "==============================================")

