#pragma once
/**
 * @file Vectors.hpp
 * @brief Defines basic vector structures for the R_Graphic namespace.
 *
 * This header provides simple 2D and 3D vector types with integer and double precision,
 * used for mathematical operations and graphics calculations in the engine.
 */
namespace R_Graphic
{
    struct vec3 {
        int x;
        int y;
        int z;
    };
    struct intVec2 {
        intVec2() = default;
        intVec2(int x1, int y1) {x = x1; y = y1;}
        int x;
        int y;
    };
    struct doubleVec2 {
        doubleVec2() = default;
        doubleVec2(double x1, double y1) {x = x1; y = y1;}
        double x;
        double y;
    };
}
