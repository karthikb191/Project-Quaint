#include <GFX/Entities/Model.h>
#include <GFX/ResourceBuilder.h>
#include <GFX/Materials/SimpleMaterial.h>
#include <chrono>
#include <random>
#include <iostream>
#include <MemCore/GlobalMemoryOverrides.h>
#include <imgui.h>

namespace Bolt
{
    Mesh::Mesh(Quaint::IMemoryContext* context)
    : m_context(context)
    {}
    
    Mesh::Mesh(Quaint::IMemoryContext* context, uint32_t vertSize, uint32_t vertOffset, uint32_t idxSize, uint32_t idxOffset)
    : Mesh(context)
    {
        m_vertexCount = vertSize;
        m_vertexOffset = vertOffset;
        m_indexCount = idxSize;
        m_indexOffset = idxOffset;
    }

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

        // m_vertices.insertRangeAt(0,
        // {
        //     { {-p0 + rXPos, -p1 + rYPos, 0.f, 1.f}, {rCol0, rCol2, rCol3, 1.f}, {0.f, 0.f, 0.f, 0.f} },
        //     { {-p2 + rXPos, p3 + rYPos, 0.f, 1.f}, {rCol0, rCol1, rCol2, 1.f}, {0.f, 1.0f, 0.f, 0.f} },
        //     { {p4 + rXPos, -p5 + rYPos, 0.f, 1.f}, {rCol1, rCol0, rCol3, 1.f}, {1.f, 0.f, 0.f, 0.f} },
        //     { {p6 + rXPos, p7 + rYPos, 0.f, 1.f}, {rCol3, rCol2, rCol0, 1.f}, {1.f, 1.f, 0.f, 0.f} }
        // }
        // );

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
        // m_indices.insertRangeAt(0, 
        // {0, 1, 2, 2, 1, 3});

        Quaint::QVertex vertex = { {-.5f, .5f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f, 0.f, 0.f} };
        constexpr int posOffset = vertex.getPositionOffset();
        constexpr int colorOffset = vertex.getColorOffset();
        constexpr int texOffset = vertex.getTexCoordOffset();
    }

    void Mesh::setMaterial(const MaterialRef material)
    {
        m_material = material;
    }


    
    Model::Model(Quaint::IMemoryContext* context, Quaint::QName name)
    : IGFXEntity(context)
    , m_transform(Quaint::QMat4x4::Identity())
    , m_meshes(context)
    , m_materials(context)
    , m_vertices(context)
    , m_indices(context)
    , m_modelImpl(nullptr, Quaint::Deleter<IModelImpl>(context))
    {
    }

    Model::Model(Quaint::IMemoryContext* context, MeshRef& mesh)
    : Model(context)
    {
        m_meshes.pushBack(std::move(mesh));
    }

    void Model::setTranslation(const Quaint::QVec4& translation)
    {
        //TODO: Should handle scale and rotation later
        m_transform.col3 = translation;
    }

    void Model::construct()
    {
        RenderObjectBuilder builder(m_context);
        m_modelImpl = std::move(builder.buildFromModel(this));
    }
    void Model::destroy()
    {
        if(m_modelImpl.get())
        {
            m_modelImpl->destroy();
        }
    }

    void Model::addMesh(float* vertices, uint32_t numVerts
            , float* normals, uint32_t numNormals
            , int* indices, uint32_t numIndices
            , float* uvs, uint32_t numUVs
            , float scale, MaterialRef material)
    {
        assert(numNormals == numVerts && "Unsupported! Currently expects one normal per vertex");

        uint32_t vertDataOffset = m_vertices.getSize();
        uint32_t vertSize = numVerts/3;
        for(size_t i = 0; i < numVerts/3; ++i)
        {
            Quaint::QVertex vertex;
            vertex.position.x = vertices[3 * i + 0] * scale;
            vertex.position.y = vertices[3 * i + 1] * scale;
            vertex.position.z = vertices[3 * i + 2] * scale;

            uint64_t timeNow = std::chrono::system_clock::now().time_since_epoch().count();
            srand(static_cast<unsigned int>(timeNow));
            vertex.color.x = rand() / (float)RAND_MAX;
            vertex.color.y = rand() / (float)RAND_MAX;
            vertex.color.z = rand() / (float)RAND_MAX;
            vertex.color.w = 1;

            vertex.normal.x = normals[3 * i + 0];
            vertex.normal.y = normals[3 * i + 1];
            vertex.normal.z = normals[3 * i + 2];
            vertex.normal.w = 1;

            m_vertices.pushBack(vertex);
        }

        //for(int i = 0; i < numIndices; ++i)
        //{
        //    m_indices.pushBack(indices[i]);
        //}

        uint32_t requiredSize = m_indices.getSize() + numIndices;
        uint32_t indexDataOffset = m_indices.getSize();
        
        for(size_t i = 0; i < numIndices; ++i)
        {
            m_indices.pushBack(indices[i] + vertDataOffset);
        }
        
        //m_indices.resize(requiredSize);
        //memcpy(m_indices.getBuffer_NonConst() + indexDataOffset, indices, numIndices * 4);
        
        Bolt::Mesh* mesh = QUAINT_NEW(m_context, Bolt::Mesh, m_context
            ,  m_vertices.getSize(), vertDataOffset, numIndices, indexDataOffset);
        Bolt::MeshRef meshRef(mesh, Quaint::Deleter<Bolt::Mesh>(m_context));
        m_meshes.pushBack(std::move(meshRef));

        auto it = std::find(m_materials.begin(), m_materials.end(), material);
        if(it == m_materials.end())
        {
            m_materials.pushBack(material);
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
        if(m_modelImpl.get())
        {
            m_modelImpl->draw(scene);
        }
    }
    
    void Model::writeImgui()
    {
        char name[1024] = {'\0'};
        sprintf_s(name, "Model: %s", m_name.getBuffer());
        if (ImGui::CollapsingHeader(name))
        {
            for(size_t i = 0; i < m_materials.getSize(); ++i)
            {
                if(ImGui::TreeNode("Material: ", "%d", i))
                {
                    m_materials[i]->writeImgui();
                    ImGui::TreePop();
                }
            }
        }
    }
    
    FloorModel::FloorModel(Quaint::IMemoryContext* context, float scale, const Quaint::QName& name)
    : Model(context, name)
    {
        //Floor is centered at 0;
        Quaint::QVertex vertex;

        vertex.position = {-0.5, 0.0, 0.5, 1};
        vertex.position *= scale;
        vertex.position.w = 1;
        vertex.normal = {0, 1, 0, 0};
        vertex.texCoord = {0, 0, 0, 0};
        m_vertices.pushBack(vertex);

        vertex.position = {0.5, 0.0, 0.5, 1};
        vertex.position *= scale;
        vertex.position.w = 1;
        vertex.normal = {0, 1, 0, 0};
        vertex.texCoord = {0, 0, 0, 0};
        m_vertices.pushBack(vertex);

        vertex.position = {0.5, 0.0, -0.5, 1};
        vertex.position *= scale;
        vertex.position.w = 1;
        vertex.normal = {0, 1, 0, 0};
        vertex.texCoord = {0, 0, 0, 0};
        m_vertices.pushBack(vertex);

        vertex.position = {-0.5, 0.0, -0.5, 1};
        vertex.position *= scale;
        vertex.position.w = 1;
        vertex.normal = {0, 1, 0, 0};
        vertex.texCoord = {0, 0, 0, 0};
        m_vertices.pushBack(vertex);
        
        m_indices.pushBack(0);
        m_indices.pushBack(1);
        m_indices.pushBack(2);
        
        m_indices.pushBack(0);
        m_indices.pushBack(2);
        m_indices.pushBack(3);

        Bolt::MaterialRef simpleMaterial = Quaint::makeShared<Bolt::SimpleMaterial>(context);
        simpleMaterial.reset(QUAINT_NEW(context, Bolt::SimpleMaterial, context));
        m_materials.pushBack(simpleMaterial);
        Bolt::Mesh* mesh = QUAINT_NEW(m_context, Bolt::Mesh, m_context
            ,  4, 0, m_indices.getSize(), 0);
        mesh->setMaterial(simpleMaterial);

        Bolt::MeshRef meshRef(mesh, Quaint::Deleter<Bolt::Mesh>(m_context));
        m_meshes.pushBack(std::move(meshRef));
    }
}