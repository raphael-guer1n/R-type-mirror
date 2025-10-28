if(DEFINED ENGINE_MODULES)
    string(FIND "${ENGINE_MODULES}" "renderer" HAS_RENDERER)
    string(FIND "${ENGINE_MODULES}" "audio" HAS_AUDIO)
    string(FIND "${ENGINE_MODULES}" "network" HAS_NETWORK)
    string(FIND "${ENGINE_MODULES}" "ecs" HAS_ECS)
    string(FIND "${ENGINE_MODULES}" "events" HAS_EVENTS)

    if(HAS_RENDERER GREATER_EQUAL 0)
        set(ENGINE_RENDERER ON)
    else()
        set(ENGINE_RENDERER OFF)
    endif()

    if(HAS_AUDIO GREATER_EQUAL 0)
        set(ENGINE_AUDIO ON)
    else()
        set(ENGINE_AUDIO OFF)
    endif()

    if(HAS_NETWORK GREATER_EQUAL 0)
        set(ENGINE_NETWORK ON)
    else()
        set(ENGINE_NETWORK OFF)
    endif()

    if(HAS_ECS GREATER_EQUAL 0)
        set(ENGINE_ECS ON)
    else()
        set(ENGINE_ECS OFF)
    endif()

    if(HAS_EVENTS GREATER_EQUAL 0)
        set(ENGINE_EVENTS ON)
    else()
        set(ENGINE_EVENTS OFF)
    endif()
endif()

option(ENGINE_RENDERER "Enable renderer subsystem (SDL-based graphics)" ON)
option(ENGINE_AUDIO "Enable audio subsystem" ON)
option(ENGINE_NETWORK "Enable network subsystem (ASIO-based)" ON)
option(ENGINE_ECS "Enable ECS subsystem" ON)
option(ENGINE_EVENTS "Enable events subsystem" ON)

set(ENGINE_CORE_SOURCES)

set(ENGINE_SOURCES)

if(ENGINE_RENDERER)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    
    list(APPEND ENGINE_SOURCES
        renderer/App.cpp
        renderer/Renderer.cpp
        renderer/Texture.cpp
        renderer/Window.cpp
    )
    
    message(STATUS "Engine: Renderer subsystem enabled")
endif()

if(ENGINE_AUDIO)
    list(APPEND ENGINE_SOURCES
        audio/Music.cpp
        audio/AudioManager.cpp
    )
    
    message(STATUS "Engine: Audio subsystem enabled")
endif()

if(ENGINE_NETWORK)
    find_package(asio CONFIG REQUIRED)
    
    list(APPEND ENGINE_SOURCES
        network/IoContext.cpp
        network/UdpSocket.cpp
    )
    
    message(STATUS "Engine: Network subsystem enabled")
endif()

if(ENGINE_ECS)
    message(STATUS "Engine: ECS subsystem enabled (header-only)")
endif()

if(ENGINE_EVENTS)
    message(STATUS "Engine: Events subsystem enabled (header-only)")
endif()

add_library(engine ${ENGINE_CORE_SOURCES} ${ENGINE_SOURCES})

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(ENGINE_RENDERER)
    target_include_directories(engine
        PUBLIC
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${SDL2_TTF_INCLUDE_DIRS}
    )
    
    target_link_directories(engine
        PUBLIC
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
    )
    
    target_link_libraries(engine
        PUBLIC
            $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
            $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
            $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
            $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    )
    
    target_compile_definitions(engine PUBLIC ENGINE_HAS_RENDERER)
endif()

if(ENGINE_AUDIO)
    target_include_directories(engine
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/audio/thirdparty
    )
    
    target_compile_definitions(engine PUBLIC ENGINE_HAS_AUDIO)
endif()

if(ENGINE_NETWORK)
    target_link_libraries(engine
        PUBLIC
            asio::asio
    )
    
    target_compile_definitions(engine PUBLIC ENGINE_HAS_NETWORK)
endif()

if(ENGINE_ECS)
    target_compile_definitions(engine PUBLIC ENGINE_HAS_ECS)
endif()

if(ENGINE_EVENTS)
    target_compile_definitions(engine PUBLIC ENGINE_HAS_EVENTS)
endif()
