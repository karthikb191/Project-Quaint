#ifndef _H_RESOURCE_BUILDER
#define _H_RESOURCE_BUILDER
#include "Entities/Resources.h"

//TODO: Rename file to VulkanResourceBuilder.cpp
namespace Bolt
{
    class ResourceBuilderFactory
    {
    public:
        template<typename T>
        static T createBuilder(Quaint::IMemoryContext* context) { return T(context); }

    private:
        ResourceBuilderFactory() = delete;
    };

//====================================================================================
    class GraphicsResourceBuilderBase
    {
    public:
        GraphicsResourceBuilderBase(Quaint::IMemoryContext* context)
        : m_context(context)
        {}

    protected:
        Quaint::IMemoryContext* m_context;
    };

    class CombinedImageSamplerTextureBuilder : public GraphicsResourceBuilderBase
    {
    public:
        CombinedImageSamplerTextureBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        {}
        //CombinedImageSamplerTextureBuilder& setSamplerInfo(); TODO: APP level sampler info structure
        
        GraphicsResource* buildFromPath(const char* path);

    private:

    };
//====================================================================================
//====================================================================================
    class BufferResourceBuilder : public GraphicsResourceBuilderBase
    {
    public:
        BufferResourceBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        {}
        BufferResourceBuilder& setBufferType(const EBufferType type) { m_bufferType = type; return *this; }
        BufferResourceBuilder& setBuffer(void* data) { m_data = data; return *this;}
        BufferResourceBuilder& setDataSize(const uint32_t size) { m_dataSize = size; return *this; }
        BufferResourceBuilder& setDataOffset(const uint32_t offset) { m_dataOffset = offset; return *this; }
        BufferResourceBuilder& setInitiallymapped(const bool map) { m_initiallyMapped = map; return *this; }
        BufferResourceBuilder& copyDataToBuffer(const bool shouldCopy) { m_copyDataTobuffer = shouldCopy; return *this; }

        GraphicsResource* build();

    protected:
        ResourceGPUProxy* buildBuffer();
        ResourceGPUProxy* buildVertexBuffer();
        ResourceGPUProxy* buildIndexBuffer();
        ResourceGPUProxy* buildUniformBuffer();

        EBufferType         m_bufferType = EBufferType::INVALID;
        void*               m_data = nullptr;
        uint32_t            m_dataSize = 0;
        uint32_t            m_dataOffset = 0;
        bool                m_initiallyMapped = false;
        bool                m_copyDataTobuffer = false;
    };
//====================================================================================
//====================================================================================
    class ShaderGroupResourceBuilder : public GraphicsResourceBuilderBase
    {
    public:
        ShaderGroupResourceBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        , m_attachmentsRefs(context)
        {}
        ShaderGroupResourceBuilder& setVertShaderPath(const char* path) { m_vertShaderPath = path; return *this; }
        ShaderGroupResourceBuilder& setFragShaderPath(const char* path) { m_fragShaderPath = path; return *this; }
        ShaderGroupResourceBuilder& addAttchmentRef(const ShaderAttachmentInfo& info);

        GraphicsResource* build();
    private:
        Quaint::QPath                           m_vertShaderPath;
        Quaint::QPath                           m_fragShaderPath;
        Quaint::QArray<ShaderAttachmentInfo>    m_attachmentsRefs;
        //TODO: Add attachment resources somehow
    };
//====================================================================================
}

#endif //_H_RESOURCE_BUILDER