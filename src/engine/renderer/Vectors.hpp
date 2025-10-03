#pragma once

namespace engine
{
    namespace R_Graphic
    {
        struct vec3
        {
            int x;
            int y;
            int z;
        };
        struct intVec2
        {
            intVec2() = default;
            intVec2(int x1, int y1)
            {
                x = x1;
                y = y1;
            }
            int x;
            int y;
        };
        struct doubleVec2
        {
            doubleVec2() = default;
            doubleVec2(double x1, double y1)
            {
                x = x1;
                y = y1;
            }
            double x;
            double y;
        };
    }
}
