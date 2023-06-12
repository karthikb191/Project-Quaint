
#include <Math/QMat.h>

namespace Quaint
{

float determinant_mf(const QMat2x2& a)
{
    return (a.row0.x * a.row1.y) - (a.row0.y * a.row1.x); 
}

#ifndef INTRINSICS_SUPPORTED
    //1. TODO: Add copy instructions

#endif

}