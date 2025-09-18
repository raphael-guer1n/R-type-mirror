# R-Engine shared library
add_library(R-Engine SHARED
    src/Engine.cpp
)

# Public headers
target_include_directories(R-Engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Warnings
target_compile_options(R-Engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Define for export macros
target_compile_definitions(R-Engine PRIVATE R_ENGINE_BUILD)

add_subdirectory(R-Graphic)

target_link_libraries(R-Engine
    PUBLIC
        R-Graphic
)

# Install rules
include(GNUInstallDirs)

install(TARGETS R-Engine
    EXPORT R-EngineTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # .dll / .so
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # shared lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # import lib (Windows)
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT R-EngineTargets
    FILE R-EngineConfig.cmake
    NAMESPACE REngine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/R-Engine
)
