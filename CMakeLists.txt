cmake_minimum_required(VERSION 3.17)
project(R-Type LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SFML COMPONENTS system window graphics network audio CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Installation rules
install(TARGETS r-type_client DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION bin/assets/ OPTIONAL)

# Print configuration summary
message(STATUS "==============================================")
message(STATUS "R-TYPE J.A.M.E.S. Configuration Summary")
message(STATUS "==============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "SFML Found: ${SFML_FOUND}")
message(STATUS "Asio Found: ${asio_FOUND}")
message(STATUS "==============================================")
