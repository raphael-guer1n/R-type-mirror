add_executable(r-type_client
    Main.cpp
)

find_package(SFML 2.6.2 REQUIRED COMPONENTS graphics window system)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(r-type_client PRIVATE
    engine
    sfml-graphics
    sfml-window
    sfml-system
)