add_executable(r-type_client
    Main.cpp
    Rtype.cpp
    Background.cpp
    Player.cpp
)

find_package(asio CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_client PRIVATE
    engine
    asio::asio
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_ttf::SDL2_ttf
)