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
    QMat3x3 mul_mf(const QMat3x3& a, const QMat3x3& b)
    {
        QMat3x3 out;
        QMat3x3 transposed(b);
        transposed.transpose();
        
        //Col 0
        __m128 T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.x));
        __m128 T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.x));
        out.pack[0] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.x));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);

        //Col 1
        T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.y));
        T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.y));
        out.pack[1] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.y));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);

        //Col 2
        T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.z));
        T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.z));
        out.pack[2] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.z));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);

        out.transpose();
        return out;
    }
    

    QMat4x4 mul_mf(const QMat4x4& a, const QMat4x4& b)
    {
        QMat4x4 transposed(b);
        transposed.transpose();
        return QMat4x4(
            QVec4( dot_vf(a.row0, transposed.row0), dot_vf(a.row0, transposed.row1), 
            dot_vf(a.row0, transposed.row2),dot_vf(a.row0, transposed.row3)),

            QVec4( dot_vf(a.row1, transposed.row0), dot_vf(a.row1, transposed.row1), 
            dot_vf(a.row1, transposed.row2),dot_vf(a.row1, transposed.row3)),

            QVec4( dot_vf(a.row2, transposed.row0), dot_vf(a.row2, transposed.row1), 
            dot_vf(a.row2, transposed.row2),dot_vf(a.row2, transposed.row3)),

            QVec4( dot_vf(a.row3, transposed.row0), dot_vf(a.row3, transposed.row1), 
            dot_vf(a.row3, transposed.row2),dot_vf(a.row3, transposed.row3))
        );
    }


    /*This is quite slow. Room for improvement?*/
    QMat4x4 mul_mf_alt(const QMat4x4& a, const QMat4x4& b)
    {
        QMat4x4 out;
        QMat4x4 transposed(b);
        transposed.transpose();
        
        //Col 0
        __m128 T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.x));
        __m128 T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.x));
        out.pack[0] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.x));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);
        T0 = _mm_mul_ps(transposed.pack[3], _mm_set_ps1(a.row3.x));
        out.pack[0] = _mm_add_ps(out.pack[0], T0);

        //Col 1
        T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.y));
        T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.y));
        out.pack[1] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.y));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);
        T0 = _mm_mul_ps(transposed.pack[3], _mm_set_ps1(a.row3.y));
        out.pack[1] = _mm_add_ps(out.pack[1], T0);

        //Col 2
        T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.z));
        T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.z));
        out.pack[2] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.z));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);
        T0 = _mm_mul_ps(transposed.pack[3], _mm_set_ps1(a.row3.z));
        out.pack[2] = _mm_add_ps(out.pack[2], T0);

        //Col 3
        T0 = _mm_mul_ps(transposed.pack[0], _mm_set_ps1(a.row0.w));
        T1 = _mm_mul_ps(transposed.pack[1], _mm_set_ps1(a.row1.w));
        out.pack[3] = _mm_add_ps(T0, T1);
        T0 = _mm_mul_ps(transposed.pack[2], _mm_set_ps1(a.row2.w));
        out.pack[3] = _mm_add_ps(out.pack[3], T0);
        T0 = _mm_mul_ps(transposed.pack[3], _mm_set_ps1(a.row3.w));
        out.pack[3] = _mm_add_ps(out.pack[3], T0);

        out.transpose();
        return out;
    }
}

#endif //INTRINSICS_SUPPORTED