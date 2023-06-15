
#include <Math/QMat.h>

namespace Quaint
{
    QVec2 mul_mf(const QMat2x2& a, const QVec2& b)
    {
        return QVec2(
            (a.col0.x * b.x) + (a.col1.x * b.y),
            (a.col0.y * b.x) + (a.col1.y * b.y)
        );
    }
    float determinant_mf(const QMat2x2& a)
    {
        return (a.col0.x * a.col1.y) - (a.col0.y * a.col1.x); 
    }

#ifndef INTRINSICS_SUPPORTED
    //1. TODO: Add copy instructions

#endif

}