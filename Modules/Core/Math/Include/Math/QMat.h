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
    QMat2x2 transpose_mf(const QMat2x2& a);
    QMat3x3 transpose_mf(const QMat3x3& a);
    QMat4x4 transpose_mf(const QMat4x4& a);


    QMat2x2 mul_mf(const QMat2x2& a, const QMat2x2& b);
    QMat3x3 mul_mf(const QMat3x3& a, const QMat3x3& b);
    QMat4x4 mul_mf(const QMat4x4& a, const QMat4x4& b);
    QMat2x2 mul_mf(const QMat2x2& a, float b);
    QMat3x3 mul_mf(const QMat3x3& a, float b);
    QMat4x4 mul_mf(const QMat4x4& a, float b);
    QVec2 mul_mf(const QMat2x2& a, const QVec2& b);
    QVec3 mul_mf(const QMat3x3& a, const QVec3& b);
    QVec4 mul_mf(const QMat4x4& a, const QVec4& b);

    QMat4x4 mul_mf_alt(const QMat4x4& a, const QMat4x4& b);

    float determinant_mf(const QMat2x2& a);
    float determinant_mf(const QMat3x3& a);
    float determinant_mf(const QMat4x4& a);

    QMat2x2 inverse_mf(const QMat2x2& a, const QMat2x2& b);
    QMat3x3 inverse_mf(const QMat3x3& a, const QMat3x3& b);
    QMat3x3 inverse_mf(const QMat4x4& a, const QMat4x4& b);

    struct alignas(16) QMat2x2
    {
        QMat2x2() : buffer{0} {}
        QMat2x2(const QVec2& col0, const QVec2& col1)
        : col0(col0), col1(col1)
        {}
        QMat2x2(float r00, float r01, float r10, float r11)
        : buffer{r00, r10, r01, r11}
        {}
        QMat2x2(float (&rowArray)[4])
        #ifdef INTRINSICS_SUPPORTED
        : pack{rowArray[0], rowArray[2], rowArray[1], rowArray[3]}
        #else
        : buffer{rowArray[0], rowArray[2], rowArray[1], rowArray[3]}
        #endif
        {}

        QMat2x2(const QMat2x2&) = default;
        QMat2x2(QMat2x2&&) = default;
        QMat2x2& operator=(const QMat2x2&) = default;
        QMat2x2& operator=(QMat2x2&&) = default;

        union
        {
            QVec2 cols[2];
            struct
            {
                QVec2 col0;
                QVec2 col1;
            };
            float buffer[4];
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack;
        #endif
        };

        inline void transpose()
        {
            *this = transpose_mf(*this);
        }
        inline float determinant() const
        {
            return determinant_mf(*this);
        }

        inline QVec2 getRow0() const{ return QVec2(col0.x, col1.x); }
        inline QVec2 getRow1() const{ return QVec2(col0.y, col1.y); }

        friend std::ostream& operator<<(std::ostream& os, const QMat2x2& mat)
        {
            os << "\n[" << mat.col0.x << ", " << mat.col1.x << "]\n";
            os << "[" << mat.col0.y << ", " << mat.col1.y << "]\n";
            return os;
        }
        
        QMat2x2 operator+(const QMat2x2& other) const
        {
            return add_mf(QMat2x2(*this), other);
        }
        QMat2x2& operator+=(const QMat2x2& other)
        {
            add_mf(*this, other);
            return *this;
        }
        QMat2x2 operator-(const QMat2x2& other) const
        {
            return sub_mf(QMat2x2(*this), other);
        }
        QMat2x2& operator-=(const QMat2x2& other)
        {
            sub_mf(*this, other);
            return *this;
        }

        /*Myriad of multiplication operators*/
        QMat2x2 operator*(const QMat2x2& other) const
        {
            return mul_mf(*this, other);
        }
        QMat2x2& operator*=(const QMat2x2& other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QMat2x2 operator*(const float other) const
        {
            return mul_mf(*this, other);
        }
        QMat2x2& operator*=(const float other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QVec2 operator*(const QVec2& other) const
        {
            return mul_mf(*this, other);
        }

        static inline QMat2x2 Identity()
        {
            return QMat2x2
            ({
                1, 0,
                0, 1
            });
        }
    };

    struct alignas(16) QMat3x3
    {
        QMat3x3() : buffer{0} {}
        QMat3x3(const QVec3& pCol0, const QVec3& pCol1, const QVec3& pCol2)
        : col0(pCol0), col1(pCol1), col2(pCol2)
        {}
        QMat3x3(const QVec4& pCol0, const QVec4& pCol1, const QVec4& pCol2)
        : col0(pCol0), col1(pCol1), col2(pCol2)
        {}
        QMat3x3(const float (&rowArray)[9])
        : col0{rowArray[0], rowArray[2], rowArray[6], 0}
        , col1{rowArray[1], rowArray[4], rowArray[7], 0}
        , col2{rowArray[2], rowArray[5], rowArray[8], 0}
        {}
        #ifdef INTRINSICS_SUPPORTED
        QMat3x3(_f4x32 pCol0, _f4x32 pCol1, _f4x32 pCol2)
        : pack{pCol0, pCol1, pCol2} 
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
                QVec4 col0;
                QVec4 col1;
                QVec4 col2;
            };
            float   buffer[12] = {0};
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack[3]; //4 dim vector per col. Total(12 elems)
        #endif
        };

        inline void transpose()
        {
            *this = transpose_mf(*this);
        }
        inline float determinant() const
        {
            return determinant_mf(*this);
        }
        inline QVec3 getRow0(){ return QVec3(col0.x, col1.x, col2.x); }
        inline QVec3 getRow1(){ return QVec3(col0.y, col1.y, col2.y); }
        inline QVec3 getRow2(){ return QVec3(col0.z, col1.z, col2.z); }

        friend std::ostream& operator<<(std::ostream& os, const QMat3x3& mat)
        {
            os << "\n[" << mat.col0.x << ", " << mat.col1.x << ", " << mat.col2.x << "]";
            os << "\n[" << mat.col0.y << ", " << mat.col1.y << ", " << mat.col2.y << "]";
            os << "\n[" << mat.col0.z << ", " << mat.col1.z << ", " << mat.col2.z << "]\n";
            return os;
        }

        inline QMat3x3 operator+(const QMat3x3& other) const
        {
            return add_mf(QMat3x3(*this), other);
        }
        inline QMat3x3& operator+=(const QMat3x3& other)
        {
            add_mf(*this, other);
            return *this;
        }
        inline QMat3x3 operator-(const QMat3x3& other) const
        {
            return sub_mf(QMat3x3(*this), other);
        }
        inline QMat3x3& operator-=(const QMat3x3& other)
        {
            *this = sub_mf(*this, other);
            return *this;
        }

        QMat3x3 operator*(const QMat3x3& other) const
        {
            return mul_mf(*this, other);
        }
        QMat3x3& operator*=(const QMat3x3& other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QMat3x3 operator*(const float other) const
        {
            return mul_mf(*this, other);
        }
        QMat3x3& operator*=(const float other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QVec3 operator*(const QVec3& other) const
        {
            return mul_mf(*this, other);
        }

        static inline QMat3x3 Identity()
        {
            return QMat3x3
            ({
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            });
        }
    };

    struct alignas(16) QMat4x4
    {
        QMat4x4() : buffer{0} {}
        QMat4x4(const QVec4& pCol0, const QVec4& pCol1, const QVec4& pCol2, const QVec4& pCol3)
        : col0(pCol0), col1(pCol1), col2(pCol2), col3(pCol3)
        {}
        QMat4x4(const QMat3x3& pMat, const QVec4& pTranslation)
        : col0(pMat.col0), col1(pMat.col1), col2(pMat.col2), col3(pTranslation)
        {}
        QMat4x4(const float (&rowArray)[16])
        : col0{rowArray[0], rowArray[4], rowArray[8], rowArray[12]}
        , col1{rowArray[1], rowArray[5], rowArray[9], rowArray[13]}
        , col2{rowArray[2], rowArray[6], rowArray[10], rowArray[14]}
        , col3{rowArray[3], rowArray[7], rowArray[11], rowArray[15]}
        {}
        #ifdef INTRINSICS_SUPPORTED
        explicit QMat4x4(_f4x32 col0, _f4x32 col1, _f4x32 col2, _f4x32 col3)
        : pack{col0, col1, col2, col3} 
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
                QVec4 col0;
                QVec4 col1;
                QVec4 col2;
                QVec4 col3;
            };
            float buffer[16];
        #ifdef INTRINSICS_SUPPORTED
            _f4x32  pack[4]; //4 dim vector per col. Total(16 elems)
        #endif
        };

        inline void transpose()
        {
            *this = transpose_mf(*this);
        }
        inline float determinant()
        {
            return determinant_mf(*this);
        }
        inline QVec4 getRow0(){ return QVec4(col0.x, col1.x, col2.x, col3.x); }
        inline QVec4 getRow1(){ return QVec4(col0.y, col1.y, col2.y, col3.y); }
        inline QVec4 getRow2(){ return QVec4(col0.z, col1.z, col2.z, col3.z); }
        inline QVec4 getRow3(){ return QVec4(col0.w, col1.w, col2.w, col3.w); }
        
        friend std::ostream& operator<<(std::ostream& os, const QMat4x4& mat)
        {
            os << "\n[" << mat.col0.x << ", " << mat.col1.x << ", " << mat.col2.x << ", " << mat.col3.x << "]";
            os << "\n[" << mat.col0.y << ", " << mat.col1.y << ", " << mat.col2.y << ", " << mat.col3.y << "]";
            os << "\n[" << mat.col0.z << ", " << mat.col1.z << ", " << mat.col2.z << ", " << mat.col3.z << "]";
            os << "\n[" << mat.col0.w << ", " << mat.col1.w << ", " << mat.col2.w << ", " << mat.col3.w << "]\n";
            return os;
        }

        inline QMat4x4 operator+(const QMat4x4& other) const
        {
            return add_mf(QMat4x4(*this), other);
        }
        inline QMat4x4& operator+=(const QMat4x4& other)
        {
            add_mf(*this, other);
            return *this;
        }
        inline QMat4x4 operator-(const QMat4x4& other) const
        {
            return sub_mf(QMat4x4(*this), other);
        }
        inline QMat4x4& operator-=(const QMat4x4& other)
        {
            sub_mf(*this, other);
            return *this;
        }

        QMat4x4 operator*(const QMat4x4& other) const
        {
            return mul_mf(*this, other);
        }
        QMat4x4& operator*=(const QMat4x4& other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QMat4x4 operator*(const float other) const
        {
            return mul_mf(*this, other);
        }
        QMat4x4& operator*=(const float other)
        {
            *this = mul_mf(*this, other);
            return *this;
        }
        QVec4 operator*(const QVec4& other) const
        {
            return mul_mf(*this, other);
        }
        
        //Conversions
        operator QMat3x3() const
        {
            return QMat3x3(col0, col1, col2);
        }

        static inline QMat4x4 Identity()
        {
            return QMat4x4
            ({
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            });
        }
        
    };

}

#endif //_Q_H_MAT