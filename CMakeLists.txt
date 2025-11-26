cmake_minimum_required(VERSION 3.23)

project(RTypeProject LANGUAGES CXX)

# ===========================
# Options de coverage
# ===========================
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Si on n'est PAS sur MSVC et que le coverage est activé,
# on ajoute les flags GCC/Clang (--coverage).
if (ENABLE_COVERAGE AND NOT MSVC)
    message(STATUS "Coverage flags enabled for GCC/Clang")
    add_compile_options(-g -O0 --coverage)
    add_link_options(--coverage)
endif()

# Find required packages (vcpkg integration)
find_package(SFML COMPONENTS Window Graphics Network Audio CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

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

# ===========================
# Sous-répertoires
# ===========================
add_subdirectory(engine)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tests)

# ===========================
# Target run-coverage
# ===========================
# Sur Windows / MSVC : utiliser OpenCppCoverage
if (MSVC)
    add_custom_target(run-coverage
        COMMAND OpenCppCoverage
            --cover_children
            --export_type html:${CMAKE_BINARY_DIR}/coverage.html
            -- ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests with OpenCppCoverage and generating coverage.html"
    )
else()
    # Sur GCC/Clang : utiliser gcovr si disponible
    find_program(GCOVR_PATH gcovr)
    if (GCOVR_PATH)
        add_custom_target(run-coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_PATH}
                --root ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/engine
                --filter ${CMAKE_SOURCE_DIR}/client
                --exclude ${CMAKE_SOURCE_DIR}/tests
                --html ${CMAKE_BINARY_DIR}/coverage.html
                --html-details
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating coverage.html with gcovr"
        )
    else()
        message(STATUS "gcovr not found: run-coverage target (non-MSVC) will not be available")
    endif()
endif()

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
message(STATUS "Asio Found: ${asio_FOUND}")
message(STATUS "Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "==============================================")
