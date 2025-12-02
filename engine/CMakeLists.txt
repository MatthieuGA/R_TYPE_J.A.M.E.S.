add_library(r-type_engine)

target_sources(r-type_engine
  PRIVATE
    src/entity.cpp
    src/registry.cpp

  PUBLIC
    FILE_SET myHeaders
    TYPE HEADERS
    FILES
      include/entity.hpp
      include/registry.hpp
      include/sparse_array.hpp
      include/sparse_array.tpp
      include/zipper.hpp
      include/zipper.tpp
      include/indexed_zipper.hpp
      include/indexed_zipper.tpp
      include/registry.tpp
)

# Make sure the include directory is accessible
target_include_directories(r-type_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
