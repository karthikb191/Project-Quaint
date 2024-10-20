#ifndef _H_BOLT_SHADER_INFO
#define _H_BOLT_SHADER_INFO
#include <Types/QStaticString.h>
#include <Types/QArray.h>
#include <cstdint>

namespace Bolt
{
    class GraphicsResource;

    enum class EShaderStage : uint32_t
    {
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        COMPUTE = 4,
        INVALID = 0xffffff
    };
    typedef uint32_t ShaderStageFlags;

    enum class EShaderResourceType
    {
        COMBINED_IMAGE_SAMPLER,
        UNIFORM_BUFFER,
        INVALID
    };

    enum class EResourceType
    {
        SHADER,
        SHADER_GROUP,
        COMBINED_IMAGE_SAMPLER,
        BUFFER,
        Invalid
    };
    
    enum EBufferType
    {
        VERTEX,
        INDEX,
        UNIFORM,
        INVALID
    };

    enum class EPrimitiveDrawType
    {
        SIMPLE,
        INDEXED,
        INDIRECT,
        INVALID
    };

    struct ShaderResourceInfo
    {
        EShaderStage                stage = EShaderStage::INVALID;
        EShaderResourceType         type = EShaderResourceType::INVALID;
        uint32_t                    set = 0; // The set it's bound to in shader
        uint32_t                    binding = 0; // The index it's bound to in shader
        uint16_t                    count = 1; // Anything greater than 1 represents an array
        bool                        perFrame = false; // If set to true, instances could be created depending on number of frames-in-flight
    };

    struct ShaderAttachmentInfo
    {
        uint32_t                set = 0;
        uint32_t                binding = 0;
        uint16_t                count = 1; //Greater than 1 for arrays
        EShaderStage            shaderStage;
        EShaderResourceType     resourceType;
        GraphicsResource*       resource = nullptr;
    };

    struct ShaderInfo
    {
        Quaint::QPath                           vertShaderPath;
        Quaint::QPath                           fragShaderPath;
        Quaint::QArray<ShaderAttachmentInfo>    resources; //TODO: Should refactor to use Resource* 
        uint8_t                                 maxResourceSets; //TODO: Might not be the best way to keep track of max sets, but this should do for now
    };
    struct GeometryRenderInfo
    {
        EPrimitiveDrawType              drawType;
        GraphicsResource*               vertBufferResource;
        GraphicsResource*               indexBufferResource;
        GraphicsResource*               shaderGroupResource;
    };

    struct VertexInputAttribute
    {

    };
}

//                                    Resource 
//    ImageResource   ImageSamplerResource    SamplerResource     UniformBufferResource

//1. How to send inputs to each of these?

#endif //_H_BOLT_SHADER_INFO