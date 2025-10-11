find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)


add_library(engine
    renderer/App.cpp
    renderer/Renderer.cpp
    renderer/Texture.cpp
    renderer/Window.cpp
    audio/Music.cpp)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg_installed/arm64-osx/include/SDL2

)

target_link_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg_installed/arm64-osx/lib
)

target_link_libraries(engine 
    PUBLIC
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        SDL2_mixer
        ogg
        vorbis
        vorbisfile
        wavpack
)