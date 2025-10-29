cmake_minimum_required(VERSION 3.16)

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")

if(NOT EXISTS "${VCPKG_ROOT}/vcpkg" AND NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe")
    message(STATUS "vcpkg not found. Downloading vcpkg...")
    
    if(NOT EXISTS "${VCPKG_ROOT}/.git")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT}
            RESULT_VARIABLE GIT_RESULT
        )
        if(NOT GIT_RESULT EQUAL "0")
            message(FATAL_ERROR "Failed to clone vcpkg repository")
        endif()
    endif()
    
    message(STATUS "Bootstrapping vcpkg...")
    if(WIN32)
        execute_process(
            COMMAND ${VCPKG_ROOT}/bootstrap-vcpkg.bat
            WORKING_DIRECTORY ${VCPKG_ROOT}
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    else()
        execute_process(
            COMMAND ${VCPKG_ROOT}/bootstrap-vcpkg.sh
            WORKING_DIRECTORY ${VCPKG_ROOT}
            RESULT_VARIABLE BOOTSTRAP_RESULT
        )
    endif()
    
    if(NOT BOOTSTRAP_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to bootstrap vcpkg")
    endif()
    
    message(STATUS "vcpkg successfully installed and bootstrapped")
endif()

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(R-type LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(ENGINE_MODULES "renderer;audio;network;ecs;events;profiling" CACHE STRING "")

option(BUILD_CLIENT "Build the R-Type client" ON)
option(BUILD_SERVER "Build the R-Type server" ON)

add_subdirectory(src/engine)

if(BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

if(BUILD_SERVER)
    add_subdirectory(src/server)
endif()
