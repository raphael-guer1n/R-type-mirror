add_executable(r-type_client main.cpp)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/shared
)

target_link_libraries(r-type_client PRIVATE R_Graphic)