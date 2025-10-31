add_executable(r-type_client
    Main.cpp
    Rtype.cpp
    Background.cpp
    Player.cpp
    Hud.cpp
    Menu.cpp
    Enemy.cpp
    Gameover.cpp
    ../common/Accessibility.cpp
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_client PRIVATE
    engine
)