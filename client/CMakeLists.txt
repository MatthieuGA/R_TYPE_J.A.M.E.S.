add_executable(r-type_client)
target_sources(r-type_client PRIVATE
    main.cpp
)
target_link_libraries(r-type_client PRIVATE r-type_engine)

# Copy executable to project root for easy access
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:r-type_client> ${CMAKE_SOURCE_DIR}/r-type_client
    COMMENT "Copying r-type_client to project root"
)
