#ifndef _H_Q_VECTOR
#define _H_Q_VECTOR

#include <Types/QBaseTypes.h>
#include <cstring>
#include <ostream>

namespace Quaint
{
    struct QVec2; struct QVec3; struct QVec4;

    /*Adds 2 vectors and stores result in first vector. Also returns a copy*/
    QVec2 add_vf(QVec2& a, const QVec2& b);
    QVec3 add_vf(QVec3& a, const QVec3& b);
    QVec4 add_vf(QVec4& a, const QVec4& b);
    
    QVec3 add_vf(QVec3& a, const QVec2& b);
    QVec4 add_vf(QVec4& a, const QVec2& b);
    QVec4 add_vf(QVec4& a, const QVec3& b);

    /*Subtrats 2 vectors and stores result in first vector. Also returns a copy*/
    QVec2 sub_vf(QVec2& a, const QVec2& b);
    QVec3 sub_vf(QVec3& a, const QVec3& b);
    QVec4 sub_vf(QVec4& a, const QVec4& b);
    
    QVec3 sub_vf(QVec3& a, const QVec2& b);
    QVec4 sub_vf(QVec4& a, const QVec2& b);
    QVec4 sub_vf(QVec4& a, const QVec3& b);

    /*Dot product*/
    float dot_vf(const QVec2& a, const QVec2& b);
    float dot_vf(const QVec3& a, const QVec3& b);
    float dot_vf(const QVec4& a, const QVec4& b);

    float dot_vf(const QVec3& a, const QVec2& b);
    float dot_vf(const QVec4& a, const QVec2& b);
    float dot_vf(const QVec4& a, const QVec3& b);

    /*Cross Product*/
    QVec3 cross_vf(const QVec3& a, const QVec3& b);
    QVec4 cross_vf(const QVec4& a, const QVec4& b);

    /*Utils*/
    float sqrMagnitude_vf(const QVec2& a);
    float sqrMagnitude_vf(const QVec3& a);
    float sqrMagnitude_vf(const QVec4& a);

    inline float sqrMagnitude_vf(const QVec3& a)
    {
        return dot_vf(a, a);
    }
    inline float sqrMagnitude_vf(const QVec4& a)
    {
        return dot_vf(a, a);
    }
    

    void copy(QVec3& to, const QVec3& from);
    void copy(QVec4& to, const QVec3& from);
    void copy(QVec4& to, const QVec4& from);

    struct alignas(8) QVec2
    {
        QVec2(){}
        QVec2(float val)
        : x(val)
        , y(val)
        {}
        QVec2(float x, float y)
        : x(x)
        , y(y)
        {}
        union
        {
            struct
            {
                float x;
                float y;
            };
            float buffer[2];
        };

        friend std::ostream& operator<<(std::ostream& os, const QVec2& vec)
        {
            os << "[" << vec.x << ", " << vec.y << "]";
            return os;
        }

        float dot(const QVec2& other)
        {
            return dot_vf(*this, other);
        }
        float sqrMagnitude()
        {
            return sqrMagnitude_vf(*this);
        }
        float magnitude()
        {
            return sqrtf(sqrMagnitude());
        }



        QVec2 operator+(const QVec2& other)
        {
            return add_vf(QVec2(*this), other);
        }
        QVec2& operator+=(const QVec2& other)
        {
            add_vf(*this, other);
            return *this;
        }

        QVec2 operator-(const QVec2& other)
        {
            return sub_vf(QVec2(*this), other);
        }
        QVec2& operator-=(const QVec2& other)
        {
            sub_vf(*this, other);
            return *this;
        }

        QVec2& operator*(float scalar)
        {
            x *= scalar; y *= scalar;
            return *this;
        }

        //QVec2& operator=(const QVec2& other)
        //{
        //    std::memcpy(buffer, other.buffer, sizeof(float) * 2);
        //    return *this;
        //}
    };
    struct alignas(16) QVec3
    {
        QVec3(): x(0), y(0), z(0){}
        QVec3(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
        {}
        QVec3(float val)
        : x(val)
        , y(val)
        , z(val)
        {}
        QVec3(float (&valArray)[3])
        : x(valArray[0])
        , y(valArray[1])
        , z(valArray[2])
        {}
        QVec3(_f4x32 pPack) : pack(pPack){}
        union
        {
            struct
            {
                float x;
                float y;
                float z;
            };
            float buffer[3];
        #ifdef INTRINSICS_SUPPORTED                        
            _f4x32      pack;
        #endif
        };

        friend std::ostream& operator<<(std::ostream& os, const QVec3& vec)
        {
            os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
            return os;
        }
        
        float dot(const QVec3& other)
        {
            return dot_vf(*this, other);
        }

        float sqrMagnitude()
        {
            return sqrMagnitude_vf(*this);
        }
        float magnitude()
        {
            return sqrtf(sqrMagnitude());
        }

        QVec3 operator+(const QVec3& other)
        { 
            return add_vf(QVec3(*this), other);
        }
        QVec3& operator+=(const QVec3& other)
        {
            add_vf(*this, other);
            return *this;
        }

        QVec3 operator-(const QVec3& other)
        {
            return sub_vf(QVec3(*this), other);
        }
        QVec3& operator-=(const QVec3& other)
        {
            sub_vf(*this, other);
            return *this;
        }

        QVec3& operator*(float scalar)
        {
            x *= scalar; y *= scalar; z *= scalar;
            return *this;
        }

        //QVec3& operator=(const QVec3& other)
        //{
        //    copy(*this, other);
        //}
    };
    struct alignas(16) QVec4
    {
        QVec4() : x(0), y(0), z(0), w(1){}
        QVec4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w)
        {}
        QVec4(const QVec3& other)
        : x(other.x), y(other.y), z(other.z), w(0)
        {}
        QVec4(float (&valArray)[4])
        : x(valArray[0]), y(valArray[1]), z(valArray[2]), w(valArray[3])
        {}
        QVec4(float val)
        : x(val), y(val), z(val), w(val)
        {}
        QVec4(_f4x32 pPack) : pack(pPack){}

        QVec4(const QVec4&) = default;
        QVec4(QVec4&&) = default;
        QVec4& operator=(const QVec4&) = default;
        QVec4& operator=(QVec4&&) = default;
        union
        {
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
            float buffer[4];
        #ifdef INTRINSICS_SUPPORTED                        
            _f4x32      pack;
        #endif
        };

        friend std::ostream& operator<<(std::ostream& os, const QVec4& vec)
        {
            os << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]";
            return os;
        }

        float dot(const QVec3& other)
        {
            return dot_vf(*this, other);
        }
        float dot(const QVec4& other)
        {
            return dot_vf(*this, other);
        }

        float sqrMagnitude()
        {
            return sqrMagnitude_vf(*this);
        }
        float magnitude()
        {
            return sqrtf(sqrMagnitude());
        }

        //Remove These
        float dot_scalar(const QVec4& other)
        {
            return (x * other.x + y * other.y + z * other.z + w * other.w);
        }
        QVec4 cross_scalar(const QVec4& other)
        {
            QVec4 res(0);
        }

        QVec4 operator+(const QVec4& other)
        {
            return add_vf(QVec4(*this), other);
        }
        QVec4& operator+=(const QVec4& other)
        {
            add_vf(*this, other);
            return *this;
        }

        QVec4 operator-(const QVec4& other)
        {
            return sub_vf(QVec4(*this), other);
        }
        QVec4& operator-=(const QVec4& other)
        {
            sub_vf(*this, other);
            return *this;
        }

        QVec4& operator*(float scalar)
        {
            x *= scalar; y *= scalar; z *= scalar; w *= scalar;
            return *this;
        }

        //QVec4& operator=(const QVec4& other)
        //{
        //    copy(*this, other);
        //}
    };

    //TODO: Remove this
    inline float dot_scalar(const QVec4& a, const QVec4& b)
    {
        return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
    }

}

#endif