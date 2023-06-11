#if defined INTRINSICS_SUPPORTED && (defined QUAINT_PLATFORM_WIN32 || defined QUAINT_PLATFORM_WIN64)
#include <Math/QMat.h>

namespace Quaint
{
    inline QMat2x2 add_mf(QMat2x2& a, const QMat2x2& b)
    {
        a.pack = _mm_add_ps(a.pack, b.pack);
        return a;
    }
    inline QMat3x3 add_mf(QMat3x3& a, const QMat3x3& b)
    {
        a.pack[0] = _mm_add_ps(a.pack[0], b.pack[0]);
        a.pack[1] = _mm_add_ps(a.pack[1], b.pack[1]);
        a.pack[2] = _mm_add_ps(a.pack[2], b.pack[2]);
        return a;
    }
    inline QMat4x4 add_mf(QMat4x4& a, const QMat4x4& b)
    {
        a.pack[0] = _mm_add_ps(a.pack[0], b.pack[0]);
        a.pack[1] = _mm_add_ps(a.pack[1], b.pack[1]);
        a.pack[2] = _mm_add_ps(a.pack[2], b.pack[2]);
        a.pack[3] = _mm_add_ps(a.pack[3], b.pack[3]);
        return a;
    }

    /*Subtraction*/
    QMat2x2 sub_mf(QMat2x2& a, const QMat2x2& b)
    {
        a.pack = _mm_sub_ps(a.pack, b.pack);
        return a;
    }
    QMat3x3 sub_mf(QMat3x3& a, const QMat3x3& b)
    {
        a.pack[0] = _mm_sub_ps(a.pack[0], b.pack[0]);
        a.pack[1] = _mm_sub_ps(a.pack[1], b.pack[1]);
        a.pack[2] = _mm_sub_ps(a.pack[2], b.pack[2]);
        return a;
    }
    QMat4x4 sub_mf(QMat4x4& a, const QMat4x4& b)
    {
        a.pack[0] = _mm_sub_ps(a.pack[0], b.pack[0]);
        a.pack[1] = _mm_sub_ps(a.pack[1], b.pack[1]);
        a.pack[2] = _mm_sub_ps(a.pack[2], b.pack[2]);
        a.pack[3] = _mm_sub_ps(a.pack[3], b.pack[3]);
        return a;
    }

    /*Multiplication*/
    QMat2x2 transpose_mf(const QMat2x2& a)
    {
        return QMat2x2(_mm_shuffle_ps(a.pack, a.pack, _MM_SHUFFLE(0, 2, 1, 3)));
    }
    /*Contents of w params are undefined*/
    QMat3x3 transpose_mf(const QMat3x3& a)
    {
        __m128 T0 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(2, 0, 2, 0)); //(a1, c1, a2, c2)
        __m128 T1 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(3, 1, 3, 1)); //(b1, d1, b2, d2)
        
        QMat3x3 res(
        _mm_shuffle_ps(T0, a.pack[2], _MM_SHUFFLE(3, 0, 2, 0)),
        _mm_shuffle_ps(T1, a.pack[2], _MM_SHUFFLE(3, 1, 2, 0)),
        _mm_shuffle_ps(T0, a.pack[2], _MM_SHUFFLE(3, 2, 3, 1))
        );
        res.row0.w = a.row0.w; res.row1.w = a.row1.w;
        res.row2.w = a.row2.w;
        return res;
    }
    QMat4x4 transpose_mf(const QMat4x4& a)
    {
        __m128 R1; 
        __m128 R2; 
        __m128 R3;
        __m128 R4;
        return QMat4x4(R1, R2, R3, R4);
    }

}

#endif //INTRINSICS_SUPPORTED