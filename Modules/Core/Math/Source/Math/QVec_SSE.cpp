#if defined INTRINSICS_SUPPORTED && (defined QUAINT_PLATFORM_WIN32 || defined QUAINT_PLATFORM_WIN64)

#include <Math/QVec.h>

namespace Quaint
{
    /*Addition*/
    QVec3 add_vf(QVec3& a, const QVec3& b)
    {
        a.pack = _mm_add_ps(a.pack, b.pack);
        return a;
    }
    QVec4 add_vf(QVec4& a, const QVec4& b)
    {
        a.pack = _mm_add_ps(a.pack, b.pack);
        return a;
    }
    QVec4 add_vf(QVec4& a, const QVec3& b)
    {
        a.pack = _mm_add_ps(a.pack, b.pack);
        return a;
    }

    /*subtraction*/
    QVec3 sub_vf(QVec3& a, const QVec3& b)
    {
        a.pack = _mm_sub_ps(a.pack, b.pack);
        return a;
    }
    QVec4 sub_vf(QVec4& a, const QVec4& b)
    {
        a.pack = _mm_sub_ps(a.pack, b.pack);
        return a;
    }
    QVec4 sub_vf(QVec4& a, const QVec3& b)
    {
        a.pack = _mm_sub_ps(a.pack, b.pack);
        return a;
    }

    /*Dot Product*/
    float dot_vf(const QVec3& a, const QVec3& b)
    {
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); // (x1*x2, y1*y2, z1*z2, ?*?) 
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 2, 0, 1)); //(y1*y2, x1*x2, z1*z2, ?*?)

        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, x1*x2+x1*x2, z1*z2+z1*z2, ?);
        shuff = _mm_movehl_ps(shuff, shuff); //(z1*z2, ?*?, x1*x2, y1*y2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2, ?, ? ,?)
    }
    float dot_vf(const QVec4& a, const QVec4& b)
    {
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); // (x1*x2, y1*y2, z1*z2, w1*w2) 
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2, 3, 0, 1)); //(y1*y2, x1*x2, w1*w2, z1*z2)

        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, y1*y2+x1*x2, z1*z2+w1*w2, w1*w2+z1*z2);
        shuff = _mm_movehl_ps(tmp, tmp); //(z1*z2+w1*w2, w1*w2+z1*z2, z1*z2 + w1*w2, z1*z2+w1*w2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2 + w1*w2, ?, ? ,?)
    }
    float dot_vf(const QVec4& a, const QVec3& b)
    {
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); // (x1*x2, y1*y2, z1*z2, ?*?) 
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 2, 0, 1)); //(y1*y2, x1*x2, z1*z2, ?*?)

        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, x1*x2+x1*x2, z1*z2+z1*z2, ?);
        shuff = _mm_movehl_ps(shuff, shuff); //(z1*z2, ?*?, x1*x2, y1*y2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2, ?, ? ,?)
    }

    void copy(QVec3& to, const QVec3& from)
    {
        to.pack = _mm_load_ps(from.buffer);
    }
    void copy(QVec4& to, const QVec3& from)
    {
        to.pack = _mm_load_ps(from.buffer);
    }
    void copy(QVec4& to, const QVec4& from)
    {
        to.pack = _mm_load_ps(from.buffer);
    }

}

#endif //INTRINSICS_SUPPORTED