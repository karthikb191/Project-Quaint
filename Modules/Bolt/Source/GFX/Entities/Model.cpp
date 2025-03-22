#include <GFX/Entities/Model.h>
#include <GFX/ResourceBuilder.h>
#include <chrono>
#include <random>

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

        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);


        //std::srand((uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count());
        float rSide = (float)dist(mt)/8.f; //(float)std::rand()/((float)RAND_MAX*4); std::srand((uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count());
        float rXPos = (float)((dist(mt) * 2.f)- 1); //((((float)std::rand()/((float)RAND_MAX)) * 2.f) - 1.f)/2.f; std::srand(std::time(0));
        float rYPos = (float)((dist(mt) * 2.f)- 1);// ((((float)std::rand()/((float)RAND_MAX)) * 2.f) - 1.f)/2.f; std::srand((uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count());
        
        float p0 = rSide;
        float p1 = p0;
        float p2 = p0;
        float p3 = p0;
        float p4 = p0;
        float p5 = p0;
        float p6 = p0;
        float p7 = p0;

        float rCol0 = (float)(dist(mt)) * 0.5f;
        float rCol1 = (float)(dist(mt)) * 0.5f;
        float rCol2 = (float)(dist(mt)) * 0.5f;
        float rCol3 = (float)(dist(mt)) * 0.5f;

        m_vertices.insertRangeAt(0,
        {
            { {-p0 + rXPos, -p1 + rYPos, 0.f, 1.f}, {rCol0, rCol2, rCol3, 1.f}, {0.f, 0.f} },
            { {-p2 + rXPos, p3 + rYPos, 0.f, 1.f}, {rCol0, rCol1, rCol2, 1.f}, {0.f, 1.0f} },
            { {p4 + rXPos, -p5 + rYPos, 0.f, 1.f}, {rCol1, rCol0, rCol3, 1.f}, {1.f, 0.f} },
            { {p6 + rXPos, p7 + rYPos, 0.f, 1.f}, {rCol3, rCol2, rCol0, 1.f}, {1.f, 1.f} }
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

    void PreDraw()
    {
        /*
        
        Each API implementation should contain the required implementation overload. The default, if hit should throw an assert


        Create a UniformBufferHandler that contains a UniformBuffer. <- Takes in Shader Uniform structure Reference for validation
        Create a UniformBuffer Structure that can be bound to GPU


        roPtr->PushUniform(<Uniform buffer data>)
        roPtr->PushUniform(<Image data>)

        roPtr->UpdateUniforms(); <- Should actually issue a vulkan command to update uniforms

        */
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