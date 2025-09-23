add_executable(r-type_server Main.cpp)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/shared
)
