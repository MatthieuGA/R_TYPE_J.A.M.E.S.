add_executable(r-type_client)

target_sources(r-type_client PRIVATE
    main.cpp
    Engine/initRegistery.cpp
)
target_include_directories(r-type_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(r-type_client PRIVATE r-type_engine
    SFML::Window SFML::Graphics SFML::Network SFML::Audio SFML::System)