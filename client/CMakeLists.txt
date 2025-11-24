add_executable(r-type_client)

target_sources(r-type_client PRIVATE
    main.cpp
)
target_link_libraries(r-type_client PRIVATE r-type_engine)

# Local development - use provided SFML libraries in External/SFML
set(SFML_ROOT "${CMAKE_SOURCE_DIR}/External/SFML")
set(SFML_INCLUDE_DIR "${SFML_ROOT}/include" "${SFML_ROOT}/src")
set(SFML_LIB_DIR "${SFML_ROOT}/lib")

target_include_directories(r-type_client PRIVATE "${SFML_INCLUDE_DIR}")
target_link_directories(r-type_client PRIVATE "${SFML_LIB_DIR}")
# Ensure the small 'breach' executable can compile sources that include SFML headers
target_include_directories(r-type_client PRIVATE "${SFML_INCLUDE_DIR}")
target_link_directories(r-type_client PRIVATE "${SFML_LIB_DIR}")

# Prefer static SFML (libs with -s suffix) and define SFML_STATIC
target_compile_definitions(r-type_client PRIVATE SFML_STATIC)
target_compile_definitions(r-type_client PRIVATE SFML_STATIC)

target_link_libraries(r-type_client PRIVATE
    # Entry point helper
    debug sfml-main-d optimized sfml-main
    # SFML static libs
    debug sfml-graphics-s-d optimized sfml-graphics-s
    debug sfml-window-s-d   optimized sfml-window-s
    debug sfml-system-s-d   optimized sfml-system-s
    debug sfml-audio-s-d    optimized sfml-audio-s
    # SFML dependencies (static/third-party and system libs)
    freetype
    openal32
    flac
    vorbisenc
    vorbisfile
    vorbis
    ogg
    opengl32
    winmm
    gdi32
)
# Also link the small 'breach' executable against sfml-main so it can build when it includes SFML headers
target_link_libraries(r-type_client PRIVATE
    debug sfml-main-d optimized sfml-main
)