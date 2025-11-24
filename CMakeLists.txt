cmake_minimum_required(VERSION 3.21)
project(R_TYPE_JAMES VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# vcpkg integration (automatic with CMAKE_TOOLCHAIN_FILE)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# Find required packages
find_package(SFML REQUIRED COMPONENTS Window Graphics Network Audio System)
find_package(asio CONFIG REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Installation rules
install(TARGETS r-type_client
    RUNTIME DESTINATION bin
)

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
