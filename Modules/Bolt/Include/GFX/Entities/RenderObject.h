#ifndef _H_BOLT_RENDER_OBJECT
#define _H_BOLT_RENDER_OBJECT

#include <stdint.h>
#include <cstddef>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <QMath.h>
#include "../Interface/IShaderGroup.h"

namespace Quaint
{
    struct QVertex
    {
        Quaint::QVec4    position;
        Quaint::QVec4    color;
        Quaint::QVec2    texCoord;

        constexpr uint32_t getPositionOffset() const { return offsetof(QVertex, position); }
        constexpr uint32_t getColorOffset() const { return offsetof(QVertex, color); }
        constexpr uint32_t getTexCoordOffset() const { return offsetof(QVertex, texCoord); }
    };
}

namespace Bolt
{
    class RenderObject
    {
    public:
        virtual void draw() = 0;
        virtual void setShaderGroup(IShaderGroup* shaderGroup) { m_shaderGroup = shaderGroup; }

        const IShaderGroup* getShaderGroup() { return m_shaderGroup; }
    
    protected:
        IShaderGroup* m_shaderGroup = nullptr;

        //TODO: Add a GPU-backed object to give renderer necessary information
    };

    class Geometry : public RenderObject
    {
    public:
        Geometry(Quaint::IMemoryContext* context);

        virtual size_t getVertexCount() const { return m_vertices.getSize(); }
        virtual const Quaint::QVertex* getVertexBuffer() const { return m_vertices.getBuffer(); }

        virtual size_t getIndexCount() const { return m_indices.getSize(); }
        virtual const uint32_t* getIndexBuffer() const { return m_indices.getBuffer(); }

        void transform(const Quaint::QVec3& position, const Quaint::QVec3& rotation, Quaint::QVec3& scale);

    protected:
        Quaint::IMemoryContext* m_memoryContext = nullptr;
        Quaint::QArray<Quaint::QVertex> m_vertices = Quaint::QArray<Quaint::QVertex>::GetInvalidPlaceholder();
        Quaint::QArray<uint32_t> m_indices = Quaint::QArray<uint32_t>::GetInvalidPlaceholder();
        Quaint::QMat4x4 m_transform;
    };

    class RenderQuad : public Geometry
    {
    public:
        RenderQuad(Quaint::IMemoryContext* context);
        virtual void draw() override;
    };
}

#endif //_H_BOLT_RENDER_OBJECT