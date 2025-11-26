cmake_minimum_required(VERSION 3.20)
project(PoC_ECS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET poc_ecs)
    add_library(poc_ecs INTERFACE)
    
    target_include_directories(poc_ecs INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    target_sources(poc_ecs INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/entity.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sparse_array.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/registry.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/components.hpp
    )
endif()
