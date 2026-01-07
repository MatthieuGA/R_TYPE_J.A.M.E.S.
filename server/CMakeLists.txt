add_executable(r-type_server)
target_sources(r-type_server PRIVATE
    main.cpp
    src/server/ClientConnectionManager.cpp
    src/server/Config.cpp
    src/server/CoreComponents.cpp
    src/server/GameplayComponents.cpp
    src/server/Network.cpp
    src/server/PacketHandler.cpp
    src/server/PacketSender.cpp
    src/server/Server.cpp
    src/server/ServerInputHandler.cpp
    src/server/ServerRegisterComponents.cpp
    src/server/ServerSendSnapshots.cpp
    src/server/ServerSetupGame.cpp
    src/server/Systems/AnimationSystem.cpp
    src/server/Systems/CollidingTools.cpp
    src/server/Systems/ExplodeOnDeathSystem.cpp
    src/server/Systems/FrameBaseEventSystem.cpp
    src/server/Systems/HealthDeductionSystem.cpp
    src/server/Systems/MovementSystem.cpp
    src/server/Systems/OriginTool.cpp
    src/server/Systems/PaternMovementSystem.cpp
    src/server/Systems/PlayerLimitPlayfield.cpp
    src/server/Systems/PlayerMovementSystem.cpp
    src/server/Systems/PlayerShootSystem.cpp
    src/server/Systems/ProjectileSystem.cpp
    src/server/Systems/ServerRegisterSystems.cpp
    src/server/Systems/TimedEventSystem.cpp
    src/server/factory/FactoryActors.cpp
    src/server/factory/FactoryActorsInit.cpp
    src/server/factory/FactoryKamiFish.cpp
    src/server/factory/FactoryMermaid.cpp
    src/server/worldgen/WorldGenConfigLoader.cpp
)

# Find nlohmann_json package
find_package(nlohmann_json CONFIG REQUIRED)

# Include directories
target_include_directories(r-type_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/engine/include
    ${Boost_INCLUDE_DIRS}
)

# Link libraries (Boost components, threading support, and JSON)
target_link_libraries(r-type_server PRIVATE 
    r-type_engine
    Boost::system
    Threads::Threads
    nlohmann_json::nlohmann_json
)

# Platform-specific linking for networking
if(WIN32)
    # Windows requires ws2_32 for Boost.Asio
    target_link_libraries(r-type_server PRIVATE ws2_32 wsock32)
endif()

# Copy worldgen assets to build directory
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/worldgen
    $<TARGET_FILE_DIR:r-type_server>/assets/worldgen
    COMMENT "Copying worldgen assets to build directory"
)

# Post build copy data folder
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    $<TARGET_FILE_DIR:r-type_server>/data
)