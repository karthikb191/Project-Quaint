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

    
    struct CombinedImageSamplerInfo
    {
        Quaint::QPath imagePath;
    };

    struct UniformBufferInputInfo
    {
        int test;
    };

    template<EResourceType ResourceType>
    class ResourceTraits
    {
    public:
        typedef void INPUT_INFO_TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::COMBINED_IMAGE_SAMPLER>
    {
    public:
        typedef CombinedImageSamplerInfo INPUT_INFO_TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::UNIFORM_BUFFER>
    {
    public:
        typedef UniformBufferInputInfo INPUT_INFO_TYPE;
    };

    //Should be implemented for specific resources on API side
    class GFXResource
    {
        
    };

    template<EResourceType ResourceType
    , typename Traits = ResourceTraits<ResourceType>>
    class Resource
    {
    public:
        Resource(typename Traits::INPUT_INFO_TYPE pInfo);

        GFXResource* resource;
    };
}

//                                    Resource 
//    ImageResource   ImageSamplerResource    SamplerResource     UniformBufferResource

//1. How to send inputs to each of these?

#endif //_H_BOLT_SHADER_INFO