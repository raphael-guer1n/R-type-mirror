find_package(Threads REQUIRED)

add_executable(r-type_server
    Server.cpp
    ServerUtils.cpp
    Main.cpp
    SpawnProjectile.cpp
    HandleInput.cpp
    HandleConnect.cpp
    enemies/Boss.cpp
    enemies/Crawler.cpp
    enemies/Shooter.cpp
    register/AreaEffect.cpp
    register/Bounds.cpp
    register/Collision.cpp
    register/Gravity.cpp
    register/HealthAndSpawn.cpp
    register/ProjectileMovement.cpp
)

find_package(asio CONFIG REQUIRED)
target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    # Path to asio if it’s standalone, otherwise system headers
)

target_link_libraries(r-type_server PRIVATE
    engine
    Threads::Threads
    asio::asio
)