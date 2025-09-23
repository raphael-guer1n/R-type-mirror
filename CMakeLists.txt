cmake_minimum_required(VERSION 3.16)

# Project
project(R-type LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executables will be output into project root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add subdirectories (new structure under src/)
add_subdirectory(src/engine)
add_subdirectory(src/client)
add_subdirectory(src/server)