add_executable(r-type_server)
target_sources(r-type_server PRIVATE
    main.cpp
)
target_link_libraries(r-type_server PRIVATE r-type_engine)