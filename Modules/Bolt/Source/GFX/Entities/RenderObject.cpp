#include <GFX/Entities/RenderObject.h>
#include <GFX/Data/ShaderInfo.h>
#include <BoltRenderer.h>
#include <RenderModule.h>
#include <iostream>

#include <GFX/ResourceBuilder.h>
#include <GFX/Entities/Resources.h>
#include <BridgeFunctions.h>

namespace Bolt
{
    Geometry::Geometry(Quaint::IMemoryContext* context)
    : RenderObject(context)
    {
        assert(context != nullptr && "Invalid memory context passed");
        m_vertices = Quaint::QArray<Quaint::QVertex>(context);
        m_indices = Quaint::QArray<uint16_t>(context);
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

    void RenderQuad::destroy()
    {
        if(m_impl != nullptr)
        {
            m_impl->destroy();
        }

        //Clear resources
        m_RenderInfo.shaderGroupResource->destroy(m_context);
        m_RenderInfo.indexBufferResource->destroy(m_context);
        m_RenderInfo.vertBufferResource->destroy(m_context);

    }

    //TODO: Remove all of these from here
    void* mappedMVPBuffer;

    void RenderQuad::load()
    {
        CombinedImageSamplerTextureBuilder imageBuilder = ResourceBuilderFactory::createBuilder<CombinedImageSamplerTextureBuilder>(m_context);
        BufferResourceBuilder bufferBuilder = ResourceBuilderFactory::createBuilder<BufferResourceBuilder>(m_context);
        ShaderGroupResourceBuilder sgBuilder = ResourceBuilderFactory::createBuilder<ShaderGroupResourceBuilder>(m_context);

        //TODO: Have a way to pass along flags
        //GraphicsResource* texResource = imageBuilder.buildFromPath("C:\\Works\\Project-Quaint\\Data\\Textures\\Test\\test.jpg");
//
        //GraphicsResource* uboResource = 
        //bufferBuilder
        //.setBuffer(nullptr)
        //.setDataSize(sizeof(Quaint::UniformBufferObject))
        //.setDataOffset(0)
        //.copyDataToBuffer(false)
        //.setBufferType(EBufferType::UNIFORM)
        //.setInitiallymapped(true)
        //.build();
//
        //GraphicsResource* vertResource = 
        //bufferBuilder
        //.setBuffer((void*)m_vertices.getBuffer())
        //.setDataSize((uint32_t)m_vertices.getSize() * (uint32_t)sizeof(decltype(m_vertices)::value_type))
        //.setDataOffset(0)
        //.copyDataToBuffer(true)
        //.setBufferType(EBufferType::VERTEX)
        //.setInitiallymapped(false)
        //.build();
        //
        //GraphicsResource* indexResource = 
        //bufferBuilder
        //.setBuffer((void*)m_indices.getBuffer())
        //.setDataSize((uint32_t)m_indices.getSize() * (uint32_t)sizeof(decltype(m_indices)::value_type))
        //.setDataOffset(0)
        //.copyDataToBuffer(true)
        //.setBufferType(EBufferType::INDEX)
        //.setInitiallymapped(false)
        //.build();
//
        //ShaderAttachmentInfo uboAttachInfo;
        //ShaderAttachmentInfo diffuseTexInfo;
        ////UBO Resource
        //uboAttachInfo.set = 0;
        //uboAttachInfo.binding = 0;
        //uboAttachInfo.count = 1;
        //uboAttachInfo.shaderStage = EShaderStage::VERTEX;
        //uboAttachInfo.resourceType = EShaderResourceType::UNIFORM_BUFFER;
        //uboAttachInfo.resource = uboResource;
//
        //// Diffuse Texture resource
        //diffuseTexInfo.set = 0;
        //diffuseTexInfo.binding = 1;
        //diffuseTexInfo.count = 1;
        //diffuseTexInfo.shaderStage = EShaderStage::FRAGMENT;
        //diffuseTexInfo.resourceType = EShaderResourceType::COMBINED_IMAGE_SAMPLER;
        //diffuseTexInfo.resource = texResource;
//
        ////GraphicsResource* shaderGroupResource = 
        ////sgBuilder
        ////.setVertShaderPath("C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv")
        ////.setFragShaderPath("C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv")
        ////.addAttchmentRef(uboAttachInfo)
        ////.addAttchmentRef(diffuseTexInfo)
        ////.build();
        //
        //m_RenderInfo.drawType = EPrimitiveDrawType::INDEXED;
        //m_RenderInfo.vertBufferResource = vertResource;
        //m_RenderInfo.indexBufferResource = indexResource;
        ////m_RenderInfo.shaderGroupResource = shaderGroupResource;
        //
//
        ////TODO: Application map interface
        //mapBufferResource(uboResource, &mappedMVPBuffer);
        ////vkMapMemory(VulkanRenderer::get()->getDevice(), mvpBufferDeviceMemory, 0, sizeof(UniformBufferObject), 0, &mappedMVPBuffer);
//
        //m_impl = RenderModule::get().getBoltRenderer()->getRenderObjectBuilder()->buildRenderObjectImplFor(this);
        //m_impl->build(m_RenderInfo);
    }

    void RenderQuad::draw()
    {
        //TODO: This should probably be moved to VulkanRenderer
        const Quaint::UniformBufferObject& obj = RenderModule::get().getBoltRenderer()->getMVPMatrix();
        memcpy(mappedMVPBuffer, &obj, sizeof(Quaint::UniformBufferObject));

        m_impl->draw(m_RenderInfo);
    }

}