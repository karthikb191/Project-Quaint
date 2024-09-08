#ifndef _H_BOLT_SHADER_INFO
#define _H_BOLT_SHADER_INFO
#include <Types/QStaticString.h>
#include <Types/QArray.h>
#include <cstdint>

namespace Bolt
{
    enum class EShaderStage
    {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        COMPUTE,
        INVALID
    };

    enum class EResourceType
    {
        COMBINED_IMAGE_SAMPLER,
        UNIFORM_BUFFER,
        INVALID
    };
    
    struct ShaderResource
    {
        EShaderStage        stage = EShaderStage::INVALID;
        EResourceType       type = EResourceType::INVALID;
        uint32_t            set = 0;        //The set it's bound to in shader
        uint32_t            binding = 0;    //The index it's bound to in shader
    };

    struct ShaderInfo
    {
        Quaint::QPath                       vertShaderPath;
        Quaint::QPath                       fragShaderPath;
        Quaint::QArray<ShaderResource>      resources;
    };
}

#endif //_H_BOLT_SHADER_INFO