
#include <Math/QMat.h>

namespace Quaint
{

float determinant_mf(const QMat2x2& a)
{
    return (a.col0.x * a.col1.y) - (a.col0.y * a.col1.x); 
}

#ifndef INTRINSICS_SUPPORTED
    //1. TODO: Add copy instructions

#endif

}