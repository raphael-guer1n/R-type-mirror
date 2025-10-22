add_executable(doodle_jump
    main.cpp
)

target_include_directories(doodle_jump PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(doodle_jump PRIVATE
    engine
)
