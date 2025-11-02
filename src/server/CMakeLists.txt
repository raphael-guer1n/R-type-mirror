find_package(Threads REQUIRED)

add_executable(r-type_server
    Server.cpp
    ServerUtils.cpp
    LevelManager.cpp
    ../common/Accessibility.cpp
    Main.cpp
    SpawnProjectile.cpp
    GameLogic.cpp
    enemies/Boss.cpp
    enemies/Crawler.cpp
    enemies/Shooter.cpp
    register/AreaEffect.cpp
    register/Bounds.cpp
    register/Collision.cpp
    register/Gravity.cpp
    register/HealthAndSpawn.cpp
    register/ProjectileMovement.cpp
    Lobby/Lobby.cpp
    Lobby/LobbyManager.cpp
    HandleInput.cpp
)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_server PRIVATE
    engine
    Threads::Threads
)