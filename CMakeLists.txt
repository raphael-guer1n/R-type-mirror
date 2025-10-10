cmake_minimum_required(VERSION 3.16)

# Project
project(R-type LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define minimum Windows version to avoid defaulting to Windows 7 target
if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Executables will be output into project root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add subdirectories (new structure under src/)
add_subdirectory(src/engine)
add_subdirectory(src/client)
add_subdirectory(src/server)