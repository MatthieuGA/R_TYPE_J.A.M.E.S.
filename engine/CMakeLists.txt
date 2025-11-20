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
)
