#ifndef _H_RESOURCE_BUILDER
#define _H_RESOURCE_BUILDER
#include "Entities/Resources.h"
#include "Entities/ShaderGroup.h"
#include "./Helpers.h"
#include <Types/QUniquePtr.h>

//TODO: Update the rest to use GPU specific objects

namespace Bolt
{
    typedef Quaint::QUniquePtr<ResourceGPUProxy, Deleter<ResourceGPUProxy>> ResourceGPUProxyPtr;
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
        
        ResourceGPUProxyPtr buildFromPath(const char* path);
        ResourceGPUProxyPtr buildFromPixels(unsigned char* pixels, int width, int height);

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

        ResourceGPUProxyPtr build();

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
    typedef Quaint::QUniquePtr<ShaderGroup, Deleter<ShaderGroup>> ShaderGroupResourceBuilderPtr;
    class ShaderGroupResourceBuilder : public GraphicsResourceBuilderBase
    {
    public:
        ShaderGroupResourceBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        , m_attachmentsRefs(context)
        , m_attributeMap(context)
        , m_ptr(nullptr, Deleter<ShaderGroup>(context))
        {}

        ShaderGroupResourceBuilder& setName(const Quaint::QName& name) { m_name = name; }
        ShaderGroupResourceBuilder& setVertShaderPath(const char* path) { m_vertShaderPath = path; return *this; }
        ShaderGroupResourceBuilder& setFragShaderPath(const char* path) { m_fragShaderPath = path; return *this; }
        ShaderGroupResourceBuilder& addAttchmentRef(const ShaderAttachmentInfo& info);

        ShaderGroupResourceBuilderPtr&& build();
    private:
        Quaint::QName                                   m_name = "";
        Quaint::QPath                                   m_vertShaderPath = "";
        Quaint::QPath                                   m_fragShaderPath = "";
        Quaint::QMap<Quaint::QName, ShaderAttribute>    m_attributeMap;
        Quaint::QArray<ShaderAttachmentInfo>            m_attachmentsRefs;
        ShaderGroupResourceBuilderPtr                   m_ptr;
        //TODO: Add attachment resources somehow
    };
//====================================================================================
//====================================================================================
    class Pipeline;
    class PipelineResourceBuilder : public GraphicsResourceBuilderBase
    {
    public:
        PipelineResourceBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        , m_ptr(nullptr, Deleter<ResourceGPUProxy>(context))
        {}

        PipelineResourceBuilder& setPipelineRef(Pipeline* pipeline);
        ResourceGPUProxyPtr build();

    private:
        ResourceGPUProxyPtr       m_ptr;
        Pipeline*                 m_pipeline = nullptr;
    };

//====================================================================================
//====================================================================================

    class Model;
    class IRenderObjectImpl;
    using RenderObjectRef = Quaint::QUniquePtr<IRenderObjectImpl, Deleter<IRenderObjectImpl>>;
    class RenderObjectBuilder : public GraphicsResourceBuilderBase
    {
    public:
        RenderObjectBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        {}
        RenderObjectRef buildFromModel(Model* model);

    private:
        
    };

//====================================================================================
//====================================================================================

}

#endif //_H_RESOURCE_BUILDER