add_executable(r-type_client
    Main.cpp
    Rtype.cpp
    Background.cpp
    Player.cpp
    Hud.cpp
    Menu.cpp
)

find_package(asio CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_client PRIVATE
    engine
    asio::asio
)