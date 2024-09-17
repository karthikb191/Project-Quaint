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
        uint32_t            set = 0; // The set it's bound to in shader
        uint32_t            binding = 0; // The index it's bound to in shader
        uint16_t            count = 1; // Anything greater than 1 represents an array
        bool                perFrame = false; // If set to true, instances could be created depending on number of frames-in-flight
    };

    struct ShaderInfo
    {
        Quaint::QPath                       vertShaderPath;
        Quaint::QPath                       fragShaderPath;
        Quaint::QArray<ShaderResource>      resources;
        uint8_t                             maxResourceSets; //TODO: Might not be the best way to keep track of max sets, but this should do for now
    };
}

#endif //_H_BOLT_SHADER_INFO