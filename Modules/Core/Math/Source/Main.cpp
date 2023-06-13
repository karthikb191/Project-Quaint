#include <Module.h>
#include <MemoryModule.h>
#include <QuaintLogger.h>
#include <windows.h>
//Initialize all modules before doing anything.
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}
#include <IPCModule.h>

#include <iostream>

#include <Math/QVec.h>
#include <Math/QMat.h>
#include <chrono>
#include <time.h>

struct Test
{
    Test()
    {
        std::cout << "Constructed\n";
    }
    Test(const Test& other)
    {
        std::cout <<"Copy made\n";
        a = other.a;
    }
    int a = 100;
};

Test test(Test& test)
{
    Test tt;
    tt.a = 1000;
    return tt;
}

int main()
{
    constexpr size_t sz = sizeof(size_t);

    Test t;
    Test n = test(t);

    Quaint::QVec3 vec1(10, 10, 10);
    Quaint::QVec3 vec2(1, 2, 3);

    Quaint::QVec3 vec3 = vec1 + vec2;
    Quaint::QVec3 vec4 = vec1 + vec2 + vec3;
    Quaint::QVec3 vec5 = vec1 + vec2 + vec3 + Quaint::QVec3(1);
    std::cout << vec3 << vec4 << vec5 << "\n";

    std::cout << vec1.dot(vec2) << "\n";

    Quaint::QVec4 vector1(100, 10, 90, 0);
    Quaint::QVec4 vector2(10, 10, 90, 1);


    std::cout << vector1.dot(vec3) << "\n";

    uint32_t test = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000 ; ++i)
    {
        vector1.dot_scalar(Quaint::QVec4(100, 100, 100, 100));
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << " Scalar DOT: " << (end - start).count() << "\n";


    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000 ; i++)
    {
        vector1.dot(Quaint::QVec4(100, 100, 100, 100));
    }

    end = std::chrono::high_resolution_clock::now();
    std::cout << "SSE DOT: " << (end - start).count() << "\n";

    Quaint::QMat2x2 mat2x2(
        {
            1, 2,
            2, 1
        }
    );
    mat2x2 *= mat2x2;
    std::cout << mat2x2;

    Quaint::QMat3x3 mat3x3(
        {
            1, 2, 3,
            3, 2, 1,
            2, 1, 3
        }
    );
    
    std::cout << "Determinant is: " << Quaint::determinant_mf(mat3x3) << "\n";

    std::cout << mat3x3;
    mat3x3 *= mat3x3;
    std::cout << mat3x3;


    //Quaint::QMat4x4 mat(
    //    {
    //        1.f, 1.f, 1.f, 0.f,
    //        2.f, 2.f, 9.f, 2.f,
    //        2.f, 4.f, 8.f, 4.f,
    //        3.f, 6.f, 9.f, 3.f
    //    }
    //);
    Quaint::QMat4x4 mat(
        Quaint::QVec4(1, 2, 2, 3),
        Quaint::QVec4(1, 2, 4, 6),
        Quaint::QVec4(1, 9, 8, 9),
        Quaint::QVec4(0, 2, 4, 3)
    );

    start = std::chrono::high_resolution_clock::now();
    float det = Quaint::determinant_mf(mat);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Determinant is: " << det << " Time Taken: " << (end - start).count() << "\n";

    //std::cout << mat;
    //mat.transpose();
    std::cout << mat;
    Quaint::QMat4x4 matorig = mat;
    
    
    
    start = std::chrono::high_resolution_clock::now();
    Quaint::QMat4x4 tt;
    //for(int i = 0; i < 100; i++)
    {
        mat = mat * mat;
        end = std::chrono::high_resolution_clock::now();
    }
    std::cout << "MATRIX MULT WITH ROW VIEW: " << (end - start).count() << "\n";


    std::cout << mat;

    //start = std::chrono::high_resolution_clock::now();
    //for(int i = 0; i < 100; i++)
    //{
    //    mat = Quaint::mul_mf_alt(matorig, matorig);
    //    end = std::chrono::high_resolution_clock::now();
    //}
    //std::cout << "MATRIX MULT WITH DOT: " << (end - start).count() << "\n";


    //std::cout << mat;


    std::cout << "Hello Math module\n";
    return 0;
}