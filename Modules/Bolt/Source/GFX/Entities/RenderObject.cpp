#include <GFX/Entities/RenderObject.h>
#include <GFX/Data/ShaderInfo.h>
#include <BoltRenderer.h>
#include <RenderModule.h>
#include <iostream>

// TODO: Remove this later once interface is ready
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt
{
    Geometry::Geometry(Quaint::IMemoryContext* context)
    : RenderObject(context)
    {
        assert(context != nullptr && "Invalid memory context passed");
        m_vertices = Quaint::QArray<Quaint::QVertex>(context);
        m_indices = Quaint::QArray<uint32_t>(context);
    }

// RenderQuad  ==============================================================================

    RenderQuad::RenderQuad(Quaint::IMemoryContext* context)
    : Geometry(context)
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

    void RenderQuad::load()
    {
        ShaderInfo info{};
        info.vertShaderPath = "D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv";
        info.fragShaderPath = "D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv";

        m_impl = RenderModule::get().getBoltRenderer()->getRenderObjectBuilder()->buildRenderObjectImplFor(this);
        m_impl->build(info);

        //TODO: Access Vulkan Renderer to create backing images and mapping memory.
        //TODO: use proper interface once available

    }

    void RenderQuad::draw()
    {

    }

//==========================================================================================

}