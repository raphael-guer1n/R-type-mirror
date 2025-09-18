add_library(R-Graphic SHARED
    src/Graphic.cpp
)

target_include_directories(R-Graphic
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(R-Graphic PRIVATE R_GRAPHIC_BUILD)
