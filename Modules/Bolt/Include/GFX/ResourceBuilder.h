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
    class UniformBufferResourceBuilder : public GraphicsResourceBuilderBase
    {
    public:
        UniformBufferResourceBuilder(Quaint::IMemoryContext* context)
        : GraphicsResourceBuilderBase(context)
        {}

        GraphicsResource* buildFromData(void* data, uint32_t size);
    private:

    };
//====================================================================================
//====================================================================================
    class ShaderGroupResourceBuilder
    {
    public:

    private:
    };
//====================================================================================
}

#endif //_H_RESOURCE_BUILDER