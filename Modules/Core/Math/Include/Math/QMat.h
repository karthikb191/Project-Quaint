#ifndef _Q_H_MAT
#define _Q_H_MAT

#include "QVec.h"


namespace Quaint
{
    struct QMat2x2; struct QMat3x3; struct QMat4x4;

    /*Addition*/
    QMat2x2 add_mf(QMat2x2& a, const QMat2x2& b);
    QMat3x3 add_mf(QMat3x3& a, const QMat3x3& b);
    QMat4x4 add_mf(QMat4x4& a, const QMat4x4& b);

    /*Subtraction*/
    QMat2x2 sub_mf(QMat2x2& a, const QMat2x2& b);
    QMat3x3 sub_mf(QMat3x3& a, const QMat3x3& b);
    QMat4x4 sub_mf(QMat4x4& a, const QMat4x4& b);

    /*Multiplication*/
    void transpose_mf(QMat2x2& a);
    void transpose_mf(QMat3x3& a);
    void transpose_mf(QMat4x4& a);

    QMat3x3 mul_mf(const QMat3x3& a, const QMat3x3& b);
    QMat4x4 mul_mf(const QMat4x4& a, const QMat4x4& b);

    struct alignas(16) QMat2x2
    {
        QMat2x2() : buffer{0} {}
        QMat2x2(const QVec2& row0, const QVec2& row1)
        : row0(row0), row1(row1)
        {}
        QMat2x2(float r00, float r01, float r10, float r11)
        : buffer{r00, r01, r10, r11}
        {}
        QMat2x2(float (&valArray)[4])
        : buffer{valArray[0], valArray[1], valArray[2], valArray[3]}
        {}
        #ifdef INTRINSICS_SUPPORTED
        QMat2x2(_f4x32  pack)
        : pack(pack)
        {}
        #endif

        QMat2x2(const QMat2x2&) = default;
        QMat2x2(QMat2x2&&) = default;
        QMat2x2& operator=(const QMat2x2&) = default;
        QMat2x2& operator=(QMat2x2&&) = default;

        union
        {
            QVec2 rows[2];
            struct
            {
                QVec2 row0;
                QVec2 row1;
            };
            float buffer[4];
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack;
        #endif
        };

        void transpose()
        {
            transpose_mf(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const QMat2x2& mat)
        {
            os << "\n[" << mat.row0.x << ", " << mat.row0.y << "]\n";
            os << "[" << mat.row1.x << ", " << mat.row1.y << "]\n";
            return os;
        }
        
        QMat2x2 operator+(const QMat2x2& other)
        {
            return add_mf(QMat2x2(*this), other);
        }
        QMat2x2& operator+=(const QMat2x2& other)
        {
            add_mf(*this, other);
            return *this;
        }
        QMat2x2 operator-(const QMat2x2& other)
        {
            return sub_mf(QMat2x2(*this), other);
        }
        QMat2x2& operator-=(const QMat2x2& other)
        {
            sub_mf(*this, other);
            return *this;
        }

    };

    struct alignas(16) QMat3x3
    {
        QMat3x3() : buffer{0} {}
        QMat3x3(const QVec3& pRow0, const QVec3& pRow1, const QVec3& pRow2)
        : row0(pRow0), row1(pRow1), row2(pRow2)
        {}
        QMat3x3(const QVec4& pRow0, const QVec4& pRow1, const QVec4& pRow2)
        : row0(pRow0), row1(pRow1), row2(pRow2)
        {}
        QMat3x3(const float (&valArray)[9])
        : row0{valArray[0], valArray[1], valArray[2], 0}
        , row1{valArray[3], valArray[4], valArray[5], 0}
        , row2{valArray[6], valArray[7], valArray[8], 0}
        {}
        #ifdef INTRINSICS_SUPPORTED
        QMat3x3(_f4x32 pRow0, _f4x32 pRow1, _f4x32 pRow2)
        : pack{pRow0, pRow1, pRow2} 
        {}
        #endif

        QMat3x3(const QMat3x3&) = default;
        QMat3x3(QMat3x3&&) = default;
        QMat3x3& operator=(const QMat3x3&) = default;
        QMat3x3& operator=(QMat3x3&&) = default;

        union
        {
            struct
            {
                QVec4 row0;
                QVec4 row1;
                QVec4 row2;
            };
            float   buffer[12] = {0};
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack[3]; //4 dim vector per row. Total(12 elems)
        #endif
        };

        void transpose()
        {
            transpose_mf(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const QMat3x3& mat)
        {
            os << "\n[" << mat.row0.x << ", " << mat.row0.y << ", " << mat.row0.z << "]";
            os << "\n[" << mat.row1.x << ", " << mat.row1.y << ", " << mat.row1.z << "]";
            os << "\n[" << mat.row2.x << ", " << mat.row2.y << ", " << mat.row2.z << "]\n";
            return os;
        }

        QMat3x3 operator+(const QMat3x3& other)
        {
            return add_mf(QMat3x3(*this), other);
        }
        QMat3x3& operator+=(const QMat3x3& other)
        {
            add_mf(*this, other);
            return *this;
        }
        QMat3x3 operator-(const QMat3x3& other)
        {
            return sub_mf(QMat3x3(*this), other);
        }
        QMat3x3& operator-=(const QMat3x3& other)
        {
            sub_mf(*this, other);
            return *this;
        }
        QMat3x3 operator*(const QMat3x3& other)
        {
            return mul_mf(*this, other);
        }
        QMat3x3& operator*=(const QMat3x3& other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
    };

    struct alignas(16) QMat4x4
    {
        QMat4x4() : buffer{0} {}
        QMat4x4(const QVec4& row0, const QVec4& row1, const QVec4& row2, const QVec4& row3)
        : row0(row0), row1(row1), row2(row2), row3(row3)
        {}
        QMat4x4(const float (&valArray)[16])
        : buffer{0}
        {
            memcpy(buffer, valArray, 16 * sizeof(float));
        }
        #ifdef INTRINSICS_SUPPORTED
        explicit QMat4x4(_f4x32 row0, _f4x32 row1, _f4x32 row2, _f4x32 row3)
        : pack{row0, row1, row2, row3} 
        {}
        #endif

        QMat4x4(const QMat4x4&) = default;
        QMat4x4(QMat4x4&&) = default;
        QMat4x4& operator=(const QMat4x4&) = default;
        QMat4x4& operator=(QMat4x4&&) = default;

        union
        {
            struct
            {
                QVec4 row0;
                QVec4 row1;
                QVec4 row2;
                QVec4 row3;
            };
            float buffer[16];
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack[4]; //4 dim vector per row. Total(16 elems)
        #endif
        };

        void transpose()
        {
            transpose_mf(*this);
        }
        
        friend std::ostream& operator<<(std::ostream& os, const QMat4x4& mat)
        {
            os << "\n[" << mat.row0.x << ", " << mat.row0.y << ", " << mat.row0.z << ", " << mat.row0.w << "]";
            os << "\n[" << mat.row1.x << ", " << mat.row1.y << ", " << mat.row1.z << ", " << mat.row1.w << "]";
            os << "\n[" << mat.row2.x << ", " << mat.row2.y << ", " << mat.row2.z << ", " << mat.row2.w << "]";
            os << "\n[" << mat.row3.x << ", " << mat.row3.y << ", " << mat.row3.z << ", " << mat.row3.w << "]\n";
            return os;
        }

        QMat4x4 operator+(const QMat4x4& other)
        {
            return add_mf(QMat4x4(*this), other);
        }
        QMat4x4& operator+=(const QMat4x4& other)
        {
            add_mf(*this, other);
            return *this;
        }
        QMat4x4 operator-(const QMat4x4& other)
        {
            return sub_mf(QMat4x4(*this), other);
        }
        QMat4x4& operator-=(const QMat4x4& other)
        {
            sub_mf(*this, other);
            return *this;
        }

        QMat4x4 operator*(const QMat4x4& other)
        {
            return mul_mf(*this, other);
        }
        QMat4x4& operator*=(const QMat4x4& other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }

    };
}

#endif //_Q_H_MAT