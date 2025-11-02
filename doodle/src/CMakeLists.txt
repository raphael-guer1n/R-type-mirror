add_executable(doodle_client
    ClientMain.cpp
)

add_executable(doodle_server
    ServerMain.cpp
)

target_include_directories(doodle_client PRIVATE
    ${CMAKE_SOURCE_DIR}/doodle/src
    ${CMAKE_SOURCE_DIR}/doodle/engine
)
target_include_directories(doodle_server PRIVATE
    ${CMAKE_SOURCE_DIR}/doodle/src
    ${CMAKE_SOURCE_DIR}/doodle/engine
)

target_link_libraries(doodle_client PRIVATE
    engine
)

target_link_libraries(doodle_server PRIVATE
    engine
)
