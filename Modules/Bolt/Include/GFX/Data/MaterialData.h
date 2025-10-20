#ifndef _H_MATERIAL_DATA
#define _H_MATERIAL_DATA

#include <Math/QMat.h>

namespace Bolt
{
    struct SimpleMaterialData
    {
        Quaint::QVec4 alignas(16) ambient = {0.5f, 0.5f, 0.5f, 1};
        Quaint::QVec4 alignas(16) diffuse = {0.5f, 0.5f, 0.5f, 1};;
        Quaint::QVec4 alignas(16) specular = {0.5f, 0.5f, 0.5f, 1};
        float alignas(4) shininess = 32;
    };
};

#endif //_H_MATERIAL_DATA