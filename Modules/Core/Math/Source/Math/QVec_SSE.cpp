#if defined INTRINSICS_SUPPORTED && (defined QUAINT_PLATFORM_WIN32 || defined QUAINT_PLATFORM_WIN64)

#include <Math/QVec.h>

namespace Quaint
{
    /*Addition*/
    QVec3 add_vf(QVec3& a, const QVec3& b)
    {
        return QVec3(_mm_add_ps(a.pack, b.pack));
    }
    QVec4 add_vf(QVec4& a, const QVec4& b)
    {
        //Addition of two vectors gives a point. W should be 1
        QVec4 res(_mm_add_ps(a.pack, b.pack));
        res.w = 1.0f;
        return res;
    }
    QVec4 add_vf(QVec4& a, const QVec3& b)
    {
        //Addition of two vectors gives a point. W should be 1
        QVec4 res(_mm_add_ps(a.pack, b.pack));
        res.w = 1.0f;
        return res;
    }

    /*subtraction*/
    QVec3 sub_vf(const QVec3& a, const QVec3& b)
    {
        return QVec3(_mm_sub_ps(a.pack, b.pack));
    }
    QVec4 sub_vf(const QVec4& a, const QVec4& b)
    {
        //Subtraction of two vectors gives a direction. W should be 0
        QVec4 res(_mm_sub_ps(a.pack, b.pack));
        res.w = 0.0f;
        return res;
    }
    QVec4 sub_vf(const QVec4& a, const QVec3& b)
    {
        //Subtraction of two vectors gives a direction. W should be 0
        QVec4 res(_mm_sub_ps(a.pack, b.pack));
        res.w = 0.0f;
        return res;
    }

    /*Dot Product*/
    float dot_vf(const QVec3& a, const QVec3& b)
    {
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); //(x1*x2, y1*y2, z1*z2, ?*?)
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 2, 0, 1)); //(y1*y2, x1*x2, z1*z2, ?*?)

        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, x1*x2+x1*x2, z1*z2+z1*z2, ?);
        shuff = _mm_shuffle_ps(shuff, shuff, _MM_SHUFFLE(0, 0, 0, 2)); //(z1*z2, ?*?, x1*x2, y1*y2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2, ?, ? ,?)
    }
    float dot_vf(const QVec4& a, const QVec4& b)
    {
        //return dot_scalar(a, b);
        //return _mm_cvtss_f32(_mm_dp_ps(a.pack, b.pack, 0xff));
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); //(x1*x2, y1*y2, z1*z2, w1*w2)
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2, 3, 0, 1)); //(y1*y2, x1*x2, w1*w2, z1*z2)
        
        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, y1*y2+x1*x2, z1*z2+w1*w2, w1*w2+z1*z2);
        shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 0, 0, 2)); //(z1*z2+w1*w2, w1*w2+z1*z2, z1*z2 + w1*w2, z1*z2+w1*w2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2 + w1*w2, ?, ? ,?)
    }
    float dot_vf(const QVec4& a, const QVec3& b)
    {
        __m128 tmp = _mm_mul_ps(a.pack, b.pack); //(x1*x2, y1*y2, z1*z2, ?*?)
        __m128 shuff = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 2, 0, 1)); //(y1*y2, x1*x2, z1*z2, ?*?)

        tmp = _mm_add_ps(tmp, shuff); //(x1*x2 + y1*y2, x1*x2+x1*x2, z1*z2+z1*z2, ?);
        shuff = _mm_shuffle_ps(shuff, shuff, _MM_SHUFFLE(0, 0, 0, 2)); //(z1*z2, ?*?, x1*x2, y1*y2)
        return _mm_cvtss_f32(_mm_add_ps(shuff, tmp)); //(x1*x2 + y1*y2 + z1*z2, ?, ? ,?)
    }

    /*Cross Product*/
    //Buffer should be 16 bytes aligned
    __m128 do_cross(__m128 a, __m128 b)
    {
        __m128 T0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)); //(y1, z1, x1, w1)
        __m128 T1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2)); //(z2, x2, y2, w2)

        __m128 mul = _mm_mul_ps(T0, T1); //(y1*z2, z1*x2, x1*y2, ?)

        T0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)); //(z1, x1, y1, w1)
        T1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)); //(y2, z2, x2, w2)
        //mul will be (y2*z1, z2*x1, x2*y1, ?)
        return _mm_sub_ps(mul, _mm_mul_ps(T0, T1)); // (y1*z2-y2*z1, z1*x2-z2*x1, x1*y2*x2*y1)
    }
    QVec3 cross_vf(const QVec3& a, const QVec3& b)
    {
        return QVec3(do_cross(a.pack, b.pack));
    }
    /*This only does cross product for 3 dimensions. Cross product for 4d vectors is not possible*/
    QVec4 cross_vf(const QVec4& a, const QVec4& b)
    {
        return QVec3(do_cross(a.pack, b.pack));
    }

    /*Utils*/


    void copy(QVec3& to, const QVec3& from)
    {
        to.pack = _mm_load_ps(from.buffer);
    }
    void copy(QVec3& to, const QVec4& from)
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