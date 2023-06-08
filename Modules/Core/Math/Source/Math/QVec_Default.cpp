
#include <Math/QVec.h>
namespace Quaint
{

QVec2 add_vf(QVec2& a, const QVec2& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}
QVec3 add_vf(QVec3& a, const QVec2& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}
QVec4 add_vf(QVec4& a, const QVec2& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

QVec3 sub_vf(QVec3& a, const QVec2& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
QVec4 sub_vf(QVec4& a, const QVec2& b)
{
    a.x -= b.x;
    a.y -=b.y;
    return a;
}
QVec2 sub_vf(QVec2& a, const QVec2& b)
{
    a.x -= b.x;
    a.y -=b.y;
    return a;
}

#ifndef INTRINSICS_SUPPORTED

    //1. TODO: Add copy instructions

#endif

}