#ifndef _H_BOLT_MODEL
#define _H_BOLT_MODEL

#include <Types/QUniquePtr.h>
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
    class UniformStorage
    {
    public:
        

    private:
        
    };

    /*Mesh contains vertex information and other sort*/
    class Mesh
    {
        public:
        Mesh(Quaint::IMemoryContext* context);
        Mesh(Quaint::IMemoryContext* context, float* vertices, uint32_t numVerts, int* indices, uint32_t numIndices, float* uvs, uint32_t numUVs);

        //virtual void loadFromFile() override {}
        //virtual void destroy() {} //TODO:
        virtual uint32_t getVertexCount() const { return (uint32_t)m_vertices.getSize(); }
        uint32_t getVertexBufferSize() const { return m_vertices.getSize() * sizeof(Quaint::QVertex); }
        virtual const Quaint::QVertex* getVertexBuffer() const { return m_vertices.getBuffer(); }

        virtual uint32_t getIndexCount() const { return (uint32_t)m_indices.getSize(); }
        virtual const uint32_t* getIndexBuffer() const { return m_indices.getBuffer(); }
        uint32_t getIndexBufferSize() const { return m_indices.getSize() * sizeof(decltype(m_indices)::value_type); }
        uint8_t getIndexBufferElementSize() const { return sizeof(decltype(m_indices)::value_type); }

        void transform(const Quaint::QVec3& position, const Quaint::QVec3& rotation, Quaint::QVec3& scale);

    protected:
        Quaint::IMemoryContext* m_context = nullptr;
        Quaint::QArray<Quaint::QVertex> m_vertices = Quaint::QArray<Quaint::QVertex>::GetInvalidPlaceholder();
        Quaint::QArray<uint32_t> m_indices = Quaint::QArray<uint32_t>::GetInvalidPlaceholder();
        Quaint::QMat4x4 m_transform;
        GeometryRenderInfo  m_RenderInfo;
    };
    using MeshRef = Quaint::QUniquePtr<Mesh, Deleter<Mesh>>;

    class QuadMesh : public Mesh
    {
    public:
        QuadMesh(Quaint::IMemoryContext* context);
    };

    /* Model is a collection of meshes, materials, etc */
    class Model : public GraphicsResource
    {
    public:
        Model(Quaint::IMemoryContext* context, MeshRef& mesh);

        void draw(RenderScene* scene);
        virtual void bindToGpu() override;
        virtual void unbindFromGPU() override;
        //TODO:  This should later have material information
        //TODO: Hoe to link this to Vulkan API?

        Mesh* getMesh(){ return m_mesh.get(); }
    private:
        MeshRef m_mesh; //TODO: Extend to support multiple meshes
        Quaint::QArray<MeshRef> m_meshes;
    };
    using ModelRef = Quaint::QUniquePtr<Model, Deleter<Model>>;

}

#endif //_H_BOLT_MODEL