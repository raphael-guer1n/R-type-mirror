add_executable(r-type_client
    src/Main.cpp
    src/Rtype.cpp
    src/Background.cpp
    src/Player.cpp
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/shared
    PUBLIC
        include
)

target_link_libraries(r-type_client PRIVATE R_Graphic)