#ifndef _H_RESOURCE_INFO
#define _H_RESOURCE_INFO

#include <Types/QStaticString.h>

namespace Bolt
{
    struct CombinedImageSamplerInfo
    {
        Quaint::QPath imagePath;
    };

    struct UniformBufferInputInfo
    {
        int test;
    };
}

#endif //_H_RESOURCE_INFO