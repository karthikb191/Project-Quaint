#include <GFX/Entities/Model.h>
#include <GFX/ResourceBuilder.h>
#include <chrono>

namespace Bolt
{
    Mesh::Mesh(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_vertices(context)
    , m_indices(context)
    {}

    QuadMesh::QuadMesh(Quaint::IMemoryContext* context)
    : Mesh(context)
    {
        /* NDCoords
        (-1,-1)          (1, -1)
        
        (-1, 1)          (1, 1)
        */

        /*
        0(-.5, -.5)       2(.5, -.5)
            -------------
            |           |
            |           |
            |           |
            -------------
        1(-.5,.5)       3(.5, .5)
        */
        float p0 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p1 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p2 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p3 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p4 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p5 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p6 = (float)std::rand()/(float)RAND_MAX; std::srand((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
        float p7 = (float)std::rand()/(float)RAND_MAX;

        m_vertices.insertRangeAt(0,
        {
            { {-p0, -p1, 0.f, 1.f}, {p0, p3, p1, 1.f}, {0.f, 0.f} },
            { {-p2, p3, 0.f, 1.f}, {p1, p1, p1, 1.f}, {0.f, 1.0f} },
            { {p4, -p5, 0.f, 1.f}, {p2, p5, p7, 1.f}, {1.f, 0.f} },
            { {p6, p7, 0.f, 1.f}, {p4, p6, p4, 1.f}, {1.f, 1.f} }
        }
        );

        //m_vertices.insertRangeAt(0,
        //{
        //    { {-.5f, -.5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f} },
        //    { {-.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.0f} },
        //    { {.5f, -.5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 0.f} },
        //    { {.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f} }
        //}
        //);
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


    Model::Model(Quaint::IMemoryContext* context, MeshRef& mesh)
    : GraphicsResource(context, EResourceType::MODEL)
    , m_mesh(std::move(mesh))
    , m_meshes(context)
    {

    }

    void Model::bindToGpu()
    {
        RenderObjectBuilder builder(m_context);
        RenderObjectRef roRef = builder.buildFromModel(this);
        assignGpuProxyResource(std::move(roRef));
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