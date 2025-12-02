add_executable(r-type_server)
target_sources(r-type_server PRIVATE
    main.cpp
)

# Include Boost headers for asio and lockfree
target_include_directories(r-type_server PRIVATE ${Boost_INCLUDE_DIRS})

# Link libraries (Boost components and threading support)
target_link_libraries(r-type_server PRIVATE 
    r-type_engine
    Boost::system
    Threads::Threads
)

# Platform-specific linking for networking
if(WIN32)
    # Windows requires ws2_32 for Boost.Asio
    target_link_libraries(r-type_server PRIVATE ws2_32 wsock32)
endif()