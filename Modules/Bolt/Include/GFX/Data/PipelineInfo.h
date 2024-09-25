#ifndef _H_PIPELINE_INFO
#define _H_PIPELINE_INFO

#include <cstdint>
#include <Math/QMat.h>
#include <Types/QArray.h>
#include "ShaderInfo.h"
#include "../Entities/Resources.h"

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
        R32G32B32A32,
        R32G32B32A32_SFLOAT,
        INVALID = 0x7FFFFFFF
    };

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
        EFormat format = EFormat::R32G32B32A32;
    };
    struct VertexInputBindingInfo
    {
        EVertexInputRate inputRate = EVertexInputRate::Vertex;
        Quaint::QArray<VertexInputAttributeInfo> attributes;
    };

    struct PipelineInfo
    {
        Quaint::QVec2                               extents {256, 256};
        Quaint::QVec2                               scissor {256, 256};
        EPrimitiveTopology                          topology = EPrimitiveTopology::LINE_LIST;
        EPolygonMode                                polygonMode = EPolygonMode::FILL;
        EBlendMode                                  blendMode = EBlendMode::DISABLED;
        CustomBlendParams                           colorBlend = {}; //Should only be handled if blendMode is CUSTOM_BLEND
        CustomBlendParams                           alphaBlend = {}; //Should only be handled if blendMode is CUSTOM_BLEND
        Quaint::QArray<VertexInputBindingInfo>      inputBindings;
    };

    struct RenderInfo
    {
        PipelineInfo                pipelineInfo;
        ShaderInfo                  shaderInfo;
        Quaint::QArray<Resource*>   resources;
    };
}

#endif //_H_PIPELINE_INFO