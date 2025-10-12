#ifndef _H_LIGHT_DATA
#define _H_LIGHT_DATA

#include <Math/QVec.h>

namespace Bolt
{
    //These structures should match data defined in the shaders
    using namespace Quaint;

    struct alignas(16) GlobalLightData
    {
        alignas(16)float direction[3] = {'\0'};
        alignas(16)float color[4] = {'\0'}; 
    };

    struct PointLightData
    {
        //TODO:
    };

    struct LightsData
    {
        GlobalLightData globalLight;
    };
};
#endif //_H_LIGHT_DATA