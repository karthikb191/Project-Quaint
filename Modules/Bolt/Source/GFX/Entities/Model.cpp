#include <GFX/Entities/Model.h>

namespace Bolt
{
    Mesh::Mesh(Quaint::IMemoryContext* context)
    : m_context(context)
    {}

    QuadMesh::QuadMesh(Quaint::IMemoryContext* context)
    : Mesh(context)
    {
        /*
        0(-.5, .5)       2(.5, .5)
            -------------
            |           |
            |           |
            |           |
            -------------
        1(-.5,-.5)       3(.5, -.5)
        */
        m_vertices.insertRangeAt(0,
        {
            { {-.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f} },
            { {-.5f,-.5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.0f} },
            { {.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 0.f} },
            { {.5f, -.5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f} }
        }
        );
        //int i = 0;
        //for(auto vertex : m_vertices)
        //{
        //    std::cout << "Vertex: " << i << "\n";
        //    std::cout << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << ", " << vertex.position.w << "\n";
        //    std::cout << vertex.color.x << ", " << vertex.color.y << ", " << vertex.color.z << ", " << vertex.color.w << "\n";
        //    std::cout << vertex.texCoord.x << ", " << vertex.texCoord.y << "\n";
        //}
        m_indices.insertRangeAt(0, 
        {0, 1, 2, 2, 1, 3});

        Quaint::QVertex vertex = { {-.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f} };
        constexpr int posOffset = vertex.getPositionOffset();
        constexpr int colorOffset = vertex.getColorOffset();
        constexpr int texOffset = vertex.getTexCoordOffset();
    }

    //TODO:
    void Mesh::transform(const Quaint::QVec3& position, const Quaint::QVec3& rotation, Quaint::QVec3& scale)
    {

    }


    Model::Model(Quaint::IMemoryContext* context)
    : GraphicsResource(context, EResourceType::MODEL)
    , m_mesh(nullptr, Deleter<Mesh>(context))
    , m_meshes(nullptr)
    {

    }

    void Model::bindToGpu()
    {

    }
    void Model::unbindFromGPU()
    {
        if(m_gpuProxyPtr.get())
        {
            m_gpuProxyPtr->destroy();
        }
    }

    void Model::draw(RenderScene* scene)
    {
        if(m_gpuProxyPtr.get())
        {
            IRenderObjectImpl* roPtr = static_cast<IRenderObjectImpl*>(m_gpuProxyPtr.get());
            roPtr->draw(scene);
        }
    }
}