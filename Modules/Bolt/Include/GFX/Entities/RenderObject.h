#ifndef _H_BOLT_RENDER_OBJECT
#define _H_BOLT_RENDER_OBJECT

#include <stdint.h>
#include <cstddef>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <QMath.h>
#include "../Interface/IRenderer.h"
#include "../Interface/IShaderGroup.h"

//TODO: Remove this
#include <vulkan/vulkan.h>

namespace Quaint
{
    struct QVertex
    {
        Quaint::QVec4    position;
        Quaint::QVec4    color;
        Quaint::QVec2    texCoord;

        constexpr static uint32_t getPositionOffset() { return offsetof(QVertex, position); }
        constexpr static uint32_t getColorOffset() { return offsetof(QVertex, color); }
        constexpr static uint32_t getTexCoordOffset() { return offsetof(QVertex, texCoord); }
    };
}

namespace Bolt
{
    //TODO: Remove this
    namespace vulkan
    {
        class RenderFrameScene;
    }

    class RenderObject
    {
    public:
        RenderObject(Quaint::IMemoryContext* context)
        : m_context(context)
        {}
        virtual void load() = 0;
        virtual void draw() = 0;
        virtual void setShaderGroup(IShaderGroup* shaderGroup) { m_shaderGroup = shaderGroup; }
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }
        const IShaderGroup* getShaderGroup() { return m_shaderGroup; }

        //TODO: Remove all of these once testing is done


    protected:
        Quaint::IMemoryContext* m_context = nullptr;
        IShaderGroup* m_shaderGroup = nullptr;
        IRenderObjectImpl* m_impl = nullptr;
        //TODO: Add a GPU-backed object to give renderer necessary information
    };

    class Geometry : public RenderObject
    {
    public:
        Geometry(Quaint::IMemoryContext* context);

        virtual void load() override {}
        virtual size_t getVertexCount() const { return m_vertices.getSize(); }
        virtual const Quaint::QVertex* getVertexBuffer() const { return m_vertices.getBuffer(); }

        virtual size_t getIndexCount() const { return m_indices.getSize(); }
        virtual const uint16_t* getIndexBuffer() const { return m_indices.getBuffer(); }

        void transform(const Quaint::QVec3& position, const Quaint::QVec3& rotation, Quaint::QVec3& scale);

    protected:
        Quaint::IMemoryContext* m_memoryContext = nullptr;
        Quaint::QArray<Quaint::QVertex> m_vertices = Quaint::QArray<Quaint::QVertex>::GetInvalidPlaceholder();
        Quaint::QArray<uint16_t> m_indices = Quaint::QArray<uint16_t>::GetInvalidPlaceholder();
        Quaint::QMat4x4 m_transform;
    };

    class RenderQuad : public Geometry
    {
    public:
        RenderQuad(Quaint::IMemoryContext* context);
        virtual void load();
        virtual void draw() override;

        //TODO: Remove this
        void drawTemp(vulkan::RenderFrameScene* context);
    };

    class VulkanTexture;
    extern VulkanTexture* getTexture_Temp();
    extern VkBuffer getUBOBuffer_Temp();
}

#endif //_H_BOLT_RENDER_OBJECT