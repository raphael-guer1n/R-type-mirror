cmake_minimum_required(VERSION 3.16)

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/../vcpkg")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using vcpkg toolchain from environment: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "vcpkg not found. Some dependencies may not be available.")
    endif()
endif()

project(DoodleJump LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(ENGINE_MODULES "renderer;audio;network;ecs;events" CACHE STRING "Engine modules to build" FORCE)

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

message(STATUS "=== Doodle Jump Build Configuration ===")
message(STATUS "Engine subsystems:")
message(STATUS "  Renderer: ${ENGINE_RENDERER}")
message(STATUS "  Audio: ${ENGINE_AUDIO}")
message(STATUS "  Network: ${ENGINE_NETWORK}")
message(STATUS "  ECS: ${ENGINE_ECS}")
message(STATUS "  Events: ${ENGINE_EVENTS}")
message(STATUS "=======================================")

add_subdirectory(engine)

add_subdirectory(src)
