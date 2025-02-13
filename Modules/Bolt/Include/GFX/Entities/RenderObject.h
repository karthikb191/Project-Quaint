#ifndef _H_BOLT_RENDER_OBJECT
#define _H_BOLT_RENDER_OBJECT

#include <stdint.h>
#include <cstddef>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <QMath.h>
#include "../Interface/IRenderer.h"
#include "../Interface/IShaderGroup.h"
#include "../Data/ShaderInfo.h"
#include "Resources.h"

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
    class RenderObject
    {
    public:
        RenderObject(Quaint::IMemoryContext* context)
        : m_context(context)
        {}
        virtual void load() = 0;
        virtual void draw() = 0;
        virtual void destroy() = 0;
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
        virtual void destroy() {} //TODO:
        virtual uint32_t getVertexCount() const { return (uint32_t)m_vertices.getSize(); }
        virtual const Quaint::QVertex* getVertexBuffer() const { return m_vertices.getBuffer(); }

        virtual uint32_t getIndexCount() const { return (uint32_t)m_indices.getSize(); }
        virtual const uint16_t* getIndexBuffer() const { return m_indices.getBuffer(); }

        void transform(const Quaint::QVec3& position, const Quaint::QVec3& rotation, Quaint::QVec3& scale);

    protected:
        Quaint::IMemoryContext* m_memoryContext = nullptr;
        Quaint::QArray<Quaint::QVertex> m_vertices = Quaint::QArray<Quaint::QVertex>::GetInvalidPlaceholder();
        Quaint::QArray<uint16_t> m_indices = Quaint::QArray<uint16_t>::GetInvalidPlaceholder();
        Quaint::QMat4x4 m_transform;
        GeometryRenderInfo  m_RenderInfo;
    };

    class RenderQuad : public Geometry
    {
    public:
        RenderQuad(Quaint::IMemoryContext* context);
        virtual void load();
        virtual void draw() override;
        virtual void destroy() override;

        //TODO: Remove this
        //void drawTemp(vulkan::RenderFrameScene* context);
    };

    /* This should be client facing */
    /* Model is basically a resource */
    class Model : public GraphicsResource
    {
    public:
        Model(Quaint::IMemoryContext* context, const Quaint::QArray<ShaderAttachmentInfo>& resources);

        void build();

        //TODO:  This should later have material information
        //TODO: Hoe to link this to Vulkan API?
    private:
        Quaint::IMemoryContext* m_context;
    };
}

#endif //_H_BOLT_RENDER_OBJECT