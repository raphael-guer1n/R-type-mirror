# Engine subsystem options
option(ENGINE_RENDERER "Enable renderer subsystem (SDL-based graphics)" ON)
option(ENGINE_AUDIO "Enable audio subsystem" ON)
option(ENGINE_NETWORK "Enable network subsystem (ASIO-based)" ON)
option(ENGINE_ECS "Enable ECS subsystem" ON)
option(ENGINE_EVENTS "Enable events subsystem" ON)

# Core engine sources (always included)
set(ENGINE_CORE_SOURCES)

# Conditional subsystem sources
set(ENGINE_SOURCES)

# Find packages based on enabled subsystems
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
    # Audio uses miniaudio (header-only, works on Fedora 42)
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
    # ECS is header-only, but we can still track it
    message(STATUS "Engine: ECS subsystem enabled (header-only)")
endif()

if(ENGINE_EVENTS)
    # Events is header-only, but we can still track it
    message(STATUS "Engine: Events subsystem enabled (header-only)")
endif()

# Create the engine library
add_library(engine ${ENGINE_CORE_SOURCES} ${ENGINE_SOURCES})

# Base include directories
target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Conditional include directories and link libraries
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
    # Include miniaudio from the thirdparty directory
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

# Print configuration summary
message(STATUS "Engine configuration:")
message(STATUS "  Renderer: ${ENGINE_RENDERER}")
message(STATUS "  Audio: ${ENGINE_AUDIO}")
message(STATUS "  Network: ${ENGINE_NETWORK}")
message(STATUS "  ECS: ${ENGINE_ECS}")
message(STATUS "  Events: ${ENGINE_EVENTS}")
