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
    # Audio might need additional packages in the future
    list(APPEND ENGINE_SOURCES
        audio/Music.cpp
    )
    
    message(STATUS "Engine: Audio subsystem enabled")
endif()

if(ENGINE_NETWORK)
    # Try to find header-only asio first, then fall back to config-mode package
    find_path(ASIO_INCLUDE_DIR asio.hpp)
    if(ASIO_INCLUDE_DIR)
        set(ASIO_HEADER_ONLY_FOUND TRUE)
    else()
        find_package(asio CONFIG QUIET)
    endif()
    if(asio_FOUND OR ASIO_HEADER_ONLY_FOUND)
        list(APPEND ENGINE_SOURCES
            network/IoContext.cpp
            network/UdpSocket.cpp
        )
        if(ASIO_HEADER_ONLY_FOUND)
            set(ENGINE_ASIO_INCLUDE ${ASIO_INCLUDE_DIR})
            message(STATUS "Engine: Network subsystem enabled (header-only asio found at ${ASIO_INCLUDE_DIR})")
        else()
            message(STATUS "Engine: Network subsystem enabled (asio config found)")
        endif()
    else()
        message(FATAL_ERROR "asio not found: install Asio or set asio_DIR / ASIO_INCLUDE_DIR")
    endif()
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

# Add top-level project include directories so engine sources inside doodle can find shared headers
target_include_directories(engine
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Conditional include directories and link libraries
if(ENGINE_RENDERER)
    target_include_directories(engine
        PUBLIC
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
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

if(ENGINE_NETWORK)
    if(asio_FOUND)
        target_link_libraries(engine
            PUBLIC
                asio::asio
        )
    elseif(ASIO_HEADER_ONLY_FOUND)
        # header-only Asio: no imported target to link, just include directory
    endif()

    target_compile_definitions(engine PUBLIC ENGINE_HAS_NETWORK)
endif()

if(ENGINE_AUDIO)
    target_compile_definitions(engine PUBLIC ENGINE_HAS_AUDIO)
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
