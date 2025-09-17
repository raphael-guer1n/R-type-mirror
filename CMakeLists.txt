cmake_minimum_required(VERSION 3.16)
project(RType LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(engine)
add_subdirectory("R-type Client")
add_subdirectory("R-type Server")