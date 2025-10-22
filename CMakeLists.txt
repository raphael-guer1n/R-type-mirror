cmake_minimum_required(VERSION 3.16)

project(R-type LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(ENGINE_MODULES "renderer;audio;network;ecs;events" CACHE STRING "Engine modules to build (renderer audio network ecs events)")

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

option(BUILD_CLIENT "Build the R-Type client" ON)
option(BUILD_SERVER "Build the R-Type server" ON)

message(STATUS "=== R-Type Build Configuration ===")
message(STATUS "Engine subsystems:")
message(STATUS "  Renderer: ${ENGINE_RENDERER}")
message(STATUS "  Audio: ${ENGINE_AUDIO}")
message(STATUS "  Network: ${ENGINE_NETWORK}")
message(STATUS "  ECS: ${ENGINE_ECS}")
message(STATUS "  Events: ${ENGINE_EVENTS}")
message(STATUS "Build targets:")
message(STATUS "  Client: ${BUILD_CLIENT}")
message(STATUS "  Server: ${BUILD_SERVER}")
message(STATUS "================================")

add_subdirectory(src/engine)

if(BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

if(BUILD_SERVER)
    add_subdirectory(src/server)
endif()
