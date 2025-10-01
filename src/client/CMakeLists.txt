add_executable(r-type_client
    Main.cpp
)

find_package(SFML 2.6.1 REQUIRED COMPONENTS graphics window system audio network)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_client PRIVATE
    engine
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network
)