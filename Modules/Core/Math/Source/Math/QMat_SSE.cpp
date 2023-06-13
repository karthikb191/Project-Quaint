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
    void transpose_mf(QMat2x2& a)
    {
        a.pack = _mm_shuffle_ps(a.pack, a.pack, _MM_SHUFFLE(0, 2, 1, 3));
    }
    /*Contents of w params are undefined*/
    void transpose_mf(QMat3x3& a)
    {
        __m128 T0 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(2, 0, 2, 0)); //(a1, c1, a2, c2)
        __m128 T1 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(3, 1, 3, 1)); //(b1, d1, b2, d2)
        
        a.pack[0] = _mm_shuffle_ps(T0, a.pack[2], _MM_SHUFFLE(3, 0, 2, 0));
        a.pack[1] = _mm_shuffle_ps(T1, a.pack[2], _MM_SHUFFLE(3, 1, 2, 0));
        a.pack[2] = _mm_shuffle_ps(T0, a.pack[2], _MM_SHUFFLE(3, 2, 3, 1));
        //res.row0.w = a.row0.w; res.row1.w = a.row1.w;
        //res.row2.w = a.row2.w;
        //return res;
    }
    void transpose_mf(QMat4x4& a)
    {
        __m128 T0 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(2, 0, 2, 0)); //(a1, c1, a2, c2)
        __m128 T1 = _mm_shuffle_ps(a.pack[0], a.pack[1], _MM_SHUFFLE(3, 1, 3, 1)); //(b1, d1, b2, d2)
        __m128 T2 = _mm_shuffle_ps(a.pack[2], a.pack[3], _MM_SHUFFLE(2, 0, 2, 0)); //(a3, c3, a4, c4)
        __m128 T3 = _mm_shuffle_ps(a.pack[2], a.pack[3], _MM_SHUFFLE(3, 1, 3, 1)); //(b3, d3, b4, d4)

        a.pack[0] = _mm_shuffle_ps(T0, T2, _MM_SHUFFLE(2, 0, 2, 0)); //(a1, a2, a3, a4)
        a.pack[1] = _mm_shuffle_ps(T1, T3, _MM_SHUFFLE(2, 0, 2, 0)); //(b1, b2, b3, b4)
        a.pack[2] = _mm_shuffle_ps(T0, T2, _MM_SHUFFLE(3, 1, 3, 1)); //(c1, c2, c3, c4)
        a.pack[3] = _mm_shuffle_ps(T1, T3, _MM_SHUFFLE(3, 1, 3, 1));  //(d1, d2, d3, d4)
    }

    //TODO: Measure performance
    QMat2x2 mul_mf(const QMat2x2& a, const QMat2x2& b)
    {
        __m128 T0 = _mm_shuffle_ps(b.pack, b.pack, _MM_SHUFFLE(3, 3, 0, 0));
        __m128 T1 = _mm_shuffle_ps(b.pack, b.pack, _MM_SHUFFLE(2, 2, 1, 1));
        __m128 T2 = _mm_shuffle_ps(a.pack, a.pack, _MM_SHUFFLE(1, 0, 3, 2));

        T0 = _mm_mul_ps(a.pack, T0);
        T1 = _mm_mul_ps(T1, T2);
        return QMat2x2((float(&)[4])_mm_add_ps(T0,T1));
    }
    QMat3x3 mul_mf(const QMat3x3& a, const QMat3x3& b)
    {
        QMat3x3 out;

        __m128 T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col0.x));
        __m128 T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col0.y));
        out.pack[0] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col0.z));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);

        //Col 1
        T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col1.x));
        T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col1.y));
        out.pack[1] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col1.z));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);

        //Col 2
        T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col2.x));
        T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col2.y));
        out.pack[2] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col2.z));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);

        return out;
    }

    /*Room for improvement? Release runs quite well*/
    QMat4x4 mul_mf(const QMat4x4& a, const QMat4x4& b)
    {
        QMat4x4 out;
        //Matrix mult using col point of view.
        /*  [a1, b1, c1, d1]    [a1, b1, c1, d1] 
        *   [a2, b2, c2, d2]    [a2, b2, c2, d2]
        *   [a3, b3, c3, d3]    [a3, b3, c3, d3]
        *   [a4, b4, c4, d4]    [a4, b4, c4, d4]
        *   : (a1, a2, a3, a4) is a column and is stored linearly in memory followed by other columns
        *   a1 * a1 + b1 * a2 + ...
        *   a2 * a1 + b2 * a2 + ...
        *   a3 * a1 + b3 * a3 + ...
        *   a4 * a1 + b4 * a4 + ....
        *   Above sequence gives us a single column. 
        *   First column in first matrix is multipled by first element in second matrix
        *   Second column in first matrix is multiplied by second element in second matrix and so on
        *   Accumulation of these columns gives us the column of final matrix. Stored in out 
        */
        //Col 0
        __m128 T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col0.x));
        __m128 T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col0.y));
        out.pack[0] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col0.z));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);
        T0 = _mm_mul_ps(a.pack[3], _mm_set_ps1(b.col0.w));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);

        //Col 1
        T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col1.x));
        T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col1.y));
        out.pack[1] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col1.z));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);
        T0 = _mm_mul_ps(a.pack[3], _mm_set_ps1(b.col1.w));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);

        //Col 2
        T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col2.x));
        T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col2.y));
        out.pack[2] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col2.z));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);
        T0 = _mm_mul_ps(a.pack[3], _mm_set_ps1(b.col2.w));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);

        //Col 3
        T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.col3.x));
        T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.col3.y));
        out.pack[3] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.col3.z));
        out.pack[3] = _mm_add_ps(out.pack[3], T0);
        T0 = _mm_mul_ps(a.pack[3], _mm_set_ps1(b.col3.w));
        out.pack[3] = _mm_add_ps(out.pack[3], T0);

        return out;
    }

    QMat2x2 mul_mf(const QMat2x2& a, float b)
    {
        return QMat2x2((float(&)[4])_mm_mul_ps(a.pack, _mm_set_ps1(b)));
    }
    QMat3x3 mul_mf(const QMat3x3& a, float b)
    {
        return QMat3x3(
            _mm_mul_ps(a.col0.pack, _mm_set1_ps(b)),
            _mm_mul_ps(a.col1.pack, _mm_set1_ps(b)),
            _mm_mul_ps(a.col2.pack, _mm_set1_ps(b))
        );
    }
    QMat4x4 mul_mf(const QMat4x4& a, float b)
    {
        return QMat4x4(
            _mm_mul_ps(a.col0.pack, _mm_set1_ps(b)),
            _mm_mul_ps(a.col1.pack, _mm_set1_ps(b)),
            _mm_mul_ps(a.col2.pack, _mm_set1_ps(b)),
            _mm_mul_ps(a.col3.pack, _mm_set1_ps(b))
        );
    }
    QVec3 mul_mf(const QMat3x3& a, const QVec3& b)
    {
        __m128 res = _mm_set_ps1(0);
        __m128 T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.x));
        __m128 T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.y));
        res = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.z));
        return QVec3(_mm_add_ps(res, T0));
    }
    QVec4 mul_mf(const QMat4x4& a, const QVec4& b)
    {
        __m128 res = _mm_set_ps1(0);
        __m128 T0 = _mm_mul_ps(a.pack[0], _mm_set_ps1(b.x));
        __m128 T1 = _mm_mul_ps(a.pack[1], _mm_set_ps1(b.y));
        res = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(a.pack[2], _mm_set_ps1(b.z));
        res = _mm_add_ps(res, T0);
        T0 = _mm_mul_ps(a.pack[3], _mm_set_ps1(b.w));
        return QVec4(_mm_add_ps(res, T0));
    }
    
    /*TODO: Implement some perf measure. Slower than method above*/
    QMat4x4 mul_mf_alt(const QMat4x4& a, const QMat4x4& b)
    {
        QMat4x4 transposed(b);
        transposed.transpose();
        return QMat4x4(
            QVec4( dot_vf(a.col0, transposed.col0), dot_vf(a.col0, transposed.col1), 
            dot_vf(a.col1, transposed.col2),dot_vf(a.col0, transposed.col3)),

            QVec4( dot_vf(a.col1, transposed.col0), dot_vf(a.col1, transposed.col1), 
            dot_vf(a.col1, transposed.col2),dot_vf(a.col1, transposed.col3)),

            QVec4( dot_vf(a.col2, transposed.col0), dot_vf(a.col2, transposed.col1), 
            dot_vf(a.col2, transposed.col2),dot_vf(a.col2, transposed.col3)),

            QVec4( dot_vf(a.col3, transposed.col0), dot_vf(a.col3, transposed.col1), 
            dot_vf(a.col3, transposed.col2),dot_vf(a.col3, transposed.col3))
        );
    }

    float determinant_mf(const QMat3x3& a)
    {
        __m128 buf = _mm_mul_ps(cross_vf(a.col1, a.col2).pack, a.col0.pack);

        __m128 T0 = _mm_shuffle_ps(buf, buf, _MM_SHUFFLE(0, 0, 0, 1)); 
        __m128 T1 = _mm_shuffle_ps(buf, buf, _MM_SHUFFLE(0, 0, 0, 2));
        return _mm_cvtss_f32(_mm_add_ps(buf, _mm_add_ps(T0, T1)));
    }
    float determinant_mf(const QMat4x4& a)
    {
        QMat3x3 TMat0 = 
        {
            Quaint::QVec3(a.col1.y, a.col1.z, a.col1.w),
            Quaint::QVec3(a.col2.y, a.col2.z, a.col2.w),
            Quaint::QVec3(a.col3.y, a.col3.z, a.col3.w)
        };
        QMat3x3 TMat1 = 
        {
            Quaint::QVec3(a.col1.x, a.col1.w, a.col1.z),
            Quaint::QVec3(a.col2.x, a.col2.w, a.col2.z),
            Quaint::QVec3(a.col3.x, a.col3.w, a.col3.z)
        };
        QMat3x3 TMat2 = 
        {
            Quaint::QVec3(a.col1.x, a.col1.y, a.col1.w),
            Quaint::QVec3(a.col2.x, a.col2.y, a.col2.w),
            Quaint::QVec3(a.col3.x, a.col3.y, a.col3.w)
        };
        QMat3x3 TMat3 = 
        {
            Quaint::QVec3(a.col1.x, a.col1.z, a.col1.y),
            Quaint::QVec3(a.col2.x, a.col2.z, a.col2.y),
            Quaint::QVec3(a.col3.x, a.col3.z, a.col3.y)
        };

        __m128 buf = {determinant_mf(TMat0), determinant_mf(TMat1), 
                    determinant_mf(TMat2), determinant_mf(TMat3)};
        buf = _mm_mul_ps(a.col0.pack, buf);
        __m128 T0 = _mm_shuffle_ps(buf, buf, _MM_SHUFFLE(2, 3, 0, 1));
        T0 = _mm_add_ps(buf, T0);
        return _mm_cvtss_f32(_mm_add_ps(T0, _mm_shuffle_ps(T0, T0, _MM_SHUFFLE(0, 0, 0, 2))));
    }
}

#endif //INTRINSICS_SUPPORTED