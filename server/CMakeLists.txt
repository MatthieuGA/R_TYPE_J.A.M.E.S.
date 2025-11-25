add_executable(r-type_server)
target_sources(r-type_server PRIVATE
    main.cpp
    src/server/Config.cpp
    src/server/Network.cpp
)
target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(r-type_server PRIVATE r-type_engine)

# Copy executable to project root for easy access
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:r-type_server> ${CMAKE_SOURCE_DIR}/r-type_server
    COMMENT "Copying r-type_server to project root"
)
