add_library(r-type_engine)

target_sources(r-type_engine
  PRIVATE
    entity.cpp
    registry.cpp

  PUBLIC
    FILE_SET myHeaders
    TYPE HEADERS
    FILES
      entity.hpp
      registry.hpp
      sparse_array.hpp
)
