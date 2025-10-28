find_package(Threads REQUIRED)

add_executable(r-type_server
    Server.cpp
    ServerUtils.cpp
    Main.cpp
)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_server PRIVATE
    engine
    Threads::Threads
)