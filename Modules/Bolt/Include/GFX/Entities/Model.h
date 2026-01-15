#ifndef _H_BOLT_MODEL
#define _H_BOLT_MODEL

#include <Types/QUniquePtr.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include <GFX/Materials/Material.h>
#include "Resources.h"

namespace Quaint
{
    struct QVertex
    {
        Quaint::QVec4    position;
        Quaint::QVec4    normal;
        Quaint::QVec4    texCoord;
        Quaint::QVec4    color;


        constexpr static uint32_t getPositionOffset() { return offsetof(QVertex, position); }
        constexpr static uint32_t getNormalOffset() { return offsetof(QVertex, normal); }
        constexpr static uint32_t getTexCoordOffset() { return offsetof(QVertex, texCoord); }
        constexpr static uint32_t getColorOffset() { return offsetof(QVertex, color); }
    };
}

namespace Bolt
{

    /*Mesh just references the data that should be owned by another container*/
    class Mesh
    {
        public:
        Mesh(Quaint::IMemoryContext* context);
        Mesh(Quaint::IMemoryContext* context, float* vertices, uint32_t numVerts
            , float* normals, uint32_t numNormals
            , int* indices, uint32_t numIndices
            , float* uvs, uint32_t numUVs
            , float scale = 1.0f);

        //TODO:
        Mesh(Quaint::IMemoryContext* context, uint32_t vertSize, uint32_t vertOffset, uint32_t idxSize, uint32_t idxOffset);

        //virtual void loadFromFile() override {}
        //virtual void destroy() {} //TODO:
        uint32_t getVertexCount() const { return m_vertexCount; }
        uint32_t getVertexDataOffset() const { return m_vertexOffset; }
        uint32_t getIndexCount() const { return m_indexCount; }
        uint32_t getIndexDatOffset() const { return m_indexOffset; }

        void setMaterial(const MaterialRef material);
        MaterialRef getMaterial(){ return m_material; }

    protected:
        Quaint::IMemoryContext* m_context = nullptr;
        
        //TODO: Use these instead of arrays above.
        // Doesn't own these parameters. Owner is responsible of cleaning these up
        uint32_t m_vertexCount = 0;
        uint32_t m_vertexOffset = 0;
        uint32_t m_indexCount = 0;
        uint32_t m_indexOffset = 0;

        GeometryRenderInfo  m_RenderInfo;
        MaterialRef m_material;
    };
    using MeshRef = Quaint::QUniquePtr<Mesh, Quaint::Deleter<Mesh>>;

    class QuadMesh : public Mesh
    {
    public:
        QuadMesh(Quaint::IMemoryContext* context);
    };

    class FloorMesh : public Mesh
    {
    public:
        FloorMesh(Quaint::IMemoryContext* context);
    };

    /* Model is a collection of meshes, materials, etc */
    class Model : public IGFXEntity
    {
    public:
        Model(Quaint::IMemoryContext* context, Quaint::QName name = "");
        Model(Quaint::IMemoryContext* context, MeshRef& mesh);

        void draw(RenderScene* scene);
        virtual void construct() override;
        virtual void destroy() override;

        //TODO:  This should later have material information
        //TODO: Hoe to link this to Vulkan API?
        
        void addMesh(float* vertices, uint32_t numVerts
            , float* normals, uint32_t numNormals
            , int* indices, uint32_t numIndices
            , float* uvs, uint32_t numUVs
            , float scale = 1.0f, MaterialRef material = nullptr);
        void addMesh(MeshRef mesh);

            
        virtual uint32_t getVertexCount() const { return (uint32_t)m_vertices.getSize(); }
        uint32_t getVertexBufferSize() const { return m_vertices.getSize() * sizeof(Quaint::QVertex); }
        virtual const Quaint::QVertex* getVertexBuffer() const { return m_vertices.getBuffer(); }

        virtual uint32_t getIndexCount() const { return (uint32_t)m_indices.getSize(); }
        virtual const uint32_t* getIndexBuffer() const { return m_indices.getBuffer(); }
        uint32_t getIndexBufferSize() const { return m_indices.getSize() * sizeof(decltype(m_indices)::value_type); }
        uint8_t getIndexBufferElementSize() const { return sizeof(decltype(m_indices)::value_type); }
        
        
            //void removeMesh(MeshRef& mesh); TODO:
        //Mesh* getMesh(){ return m_mesh.get(); }
        
        void setMaterial(MaterialRef material);

        Quaint::QArray<MeshRef>& getMeshes() { return m_meshes; }
        Quaint::QArray<MaterialRef>& getMaterials() { return m_materials; }
        
        void setName(const Quaint::QName& name){ m_name = name; }
        const Quaint::QName& getName() const { return m_name; }

        void setTranslation(const Quaint::QVec4& translation);
        const Quaint::QMat4x4& getTransform() const { return m_transform; } 

        void writeImgui();
        
    protected:
        Quaint::QName m_name;
        Quaint::QMat4x4 m_transform;
        Quaint::QArray<MeshRef> m_meshes;
        Quaint::QArray<MaterialRef> m_materials;
        Quaint::QArray<Quaint::QVertex> m_vertices = Quaint::QArray<Quaint::QVertex>::GetInvalidPlaceholder();
        Quaint::QArray<uint32_t> m_indices = Quaint::QArray<uint32_t>::GetInvalidPlaceholder();
        TModelImplPtr m_modelImpl;
    };

    class FloorModel : public Model
    {
    public:
        FloorModel(Quaint::IMemoryContext* context, float scale = 1.0f, const Quaint::QName& name = "");
    };

    using ModelRef = Quaint::QUniquePtr<Model, Quaint::Deleter<Model>>;

}

#endif //_H_BOLT_MODEL