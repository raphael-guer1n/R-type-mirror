cmake_minimum_required(VERSION 3.16)
project(RType LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Place built binaries at the repo root for convenience
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Subprojects
add_subdirectory(engine)

# Allow choosing between legacy layout and main's Src/ layout
option(USE_SRC_LAYOUT "Build Client/Server from Src/ instead of R-type */" OFF)

if (USE_SRC_LAYOUT)
	# Layout from origin/main
	add_subdirectory(Src/Client)
	add_subdirectory(Src/Server)
else()
	# Current branch layout
	add_subdirectory("R-type Client")
	add_subdirectory("R-type Server")
endif()
