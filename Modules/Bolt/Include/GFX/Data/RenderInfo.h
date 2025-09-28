#ifndef _H_RENDER_INFO
#define _H_RENDER_INFO

#include <cstdint>
#include <Math/QMat.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
//#include "ShaderInfo.h"

namespace Bolt
{
    enum class EPrimitiveTopology
    {
        POINT_LIST = 0,
        LINE_LIST = 1,
        LINE_STRIP = 2,
        TRIANGLE_LIST = 3,
        TRIANGLE_STRIP = 4,
        TRIANGLE_FAN = 5,
        INVALID = 0x7FFFFFFF
    };

    enum class EPolygonMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2,
        INVALID = 0x7FFFFFFF
    };

    enum class EBlendMode
    {
        DISABLED = 0,
        ALPHA_BLEND = 1,
        CUSTOM_BLEND = 2,
        INVALID = 0x7FFFFFFF
    };
    
    enum class EFormat
    {
        R32G32B32A32_UINT,
        R32G32B32A32_SFLOAT,
        R32G32_SFLOAT,
        R8G8B8A8_SRGB,
        R8G8B8A8_UNORM,
        INVALID = 0x7FFFFFFF
    };

    enum EImageUsage : uint32_t
    {
        COLOR_ATTACHMENT = 0,
        DEPTH_ATTACHMENT = 1 << 0,
        INPUT_ATTACHMENT = 1 << 1,
        SAMPLED = 1 << 2,
        COMBINED_IMAGE_SAMPLER = 1 << 3,
        COPY_SRC = 1 << 4, // Used as source of copy operations
        COPY_DST = 1 << 5, // Used as destination of copy operations
    };
    typedef uint32_t EImageUsageFlags; 

    enum class EVertexInputRate
    {
        Vertex,
        Instance,
        Invalid
    };

    struct CustomBlendParams
    {
        uint32_t    srcBlend = 0;
        uint32_t    dstBlend = 0;
        uint32_t    blendOp = 0;
    };

    struct VertexInputAttributeInfo
    {
        uint32_t binding = -1;
        uint32_t location = -1;
        uint32_t offset = 0;
        EFormat format = EFormat::R8G8B8A8_SRGB;
    };
    struct VertexInputBindingInfo
    {
        EVertexInputRate inputRate = EVertexInputRate::Vertex;
        Quaint::QArray<VertexInputAttributeInfo> attributes;
    };

    struct ImageDefinition
    {

    };

    struct AttachmentDefinition
    {
        enum class Type { Image, Depth, Swapchain };

        Quaint::QName name = "";
        uint32_t binding = 0;
        Type type = Type::Image;
        EFormat format = EFormat::R8G8B8A8_SRGB;
        EImageUsageFlags usage = EImageUsage::COLOR_ATTACHMENT;
        Quaint::QVec4 clearColor = {1.f, 0.f, 0.5f, 1.f};
        Quaint::QVec2 extents = {256, 256};
        bool clearImage = false;
        bool storePrevious = false;
        //bool forEachSwapcahinImage = false;
    };

    struct RenderInfo
    {
        Quaint::QVec2                               extents {256, 256}; 
        Quaint::QVec2                               offset {256, 256};
        Quaint::QVec2                               scissor {256, 256};
        EPrimitiveTopology                          topology = EPrimitiveTopology::TRIANGLE_LIST;
        EPolygonMode                                polygonMode = EPolygonMode::FILL;
        EBlendMode                                  blendMode = EBlendMode::DISABLED;
        CustomBlendParams                           colorBlend = {}; //Should only be handled if blendMode is CUSTOM_BLEND
        CustomBlendParams                           alphaBlend = {}; //Should only be handled if blendMode is CUSTOM_BLEND
        Quaint::QArray<AttachmentDefinition>        attachments;
    };
}

#endif //_H_RENDER_INFO