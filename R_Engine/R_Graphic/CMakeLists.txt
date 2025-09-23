find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_library(R_Graphic
    Src/Window.cpp
    Src/Renderer.cpp
    Src/Sprite.cpp
    Src/Texture.cpp
    Src/Background.cpp
    Src/App.cpp
)

target_include_directories(R_Graphic 
    PUBLIC 
        include
    PRIVATE 
        Src
)

target_link_libraries(R_Graphic 
    PUBLIC
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

target_compile_options(R_Graphic PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
