#include <GFX/Entities/RenderObject.h>
#include <GFX/Data/ShaderInfo.h>
#include <BoltRenderer.h>
#include <RenderModule.h>
#include <iostream>

// TODO: Remove this later once interface is ready
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>


#include <GFX/ResourceBuilder.h>
#include <GFX/Entities/Resources.h>

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

    //TODO: Remove all of these from here
    VulkanTexture texture;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferDeviceMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferDeviceMemory;

    VkBuffer mvpBuffer;
    VkDeviceMemory mvpBufferDeviceMemory;
    void* mappedMVPBuffer;

    void RenderQuad::load()
    {
        CombinedImageSamplerTextureBuilder imageBuilder = ResourceBuilderFactory::createBuilder<CombinedImageSamplerTextureBuilder>(m_context);
        BufferResourceBuilder bufferBuilder = ResourceBuilderFactory::createBuilder<BufferResourceBuilder>(m_context);
        ShaderGroupResourceBuilder sgBuilder = ResourceBuilderFactory::createBuilder<ShaderGroupResourceBuilder>(m_context);

        //TODO: Have a way to pass along flags
        GraphicsResource* texResource = imageBuilder.buildFromPath("D:\\Works\\Project-Quaint\\Data\\Textures\\Test\\test.jpg");

        GraphicsResource* uboResource = 
        bufferBuilder
        .setBuffer(nullptr)
        .setDataSize(sizeof(UniformBufferObject))
        .copyDataToBuffer(false)
        .setBufferType(EBufferType::UNIFORM)
        .setInitiallymapped(true)
        .build();

        GraphicsResource* vertResource = 
        bufferBuilder
        .setBuffer((void*)m_vertices.getBuffer())
        .setDataSize(m_vertices.getSize() * sizeof(decltype(m_vertices)::value_type))
        .copyDataToBuffer(true)
        .setBufferType(EBufferType::VERTEX)
        .setInitiallymapped(false)
        .build();
        
        GraphicsResource* indexResource = 
        bufferBuilder
        .setBuffer((void*)m_indices.getBuffer())
        .setDataSize(m_indices.getSize() * sizeof(decltype(m_indices)::value_type))
        .copyDataToBuffer(true)
        .setBufferType(EBufferType::INDEX)
        .setInitiallymapped(false)
        .build();

        vulkan::VulkanCombinedImageSamplerResource* vkTexRes 
        = static_cast<vulkan::VulkanCombinedImageSamplerResource*>(texResource->getGpuResourceProxy());
        texture = vkTexRes->getTexture();

        vertexBuffer = static_cast<vulkan::VulkanBufferObjectResource*>(vertResource->getGpuResourceProxy())->getBufferhandle(); 
        indexBuffer = static_cast<vulkan::VulkanBufferObjectResource*>(indexResource->getGpuResourceProxy())->getBufferhandle();
        mvpBuffer = static_cast<vulkan::VulkanBufferObjectResource*>(uboResource->getGpuResourceProxy())->getBufferhandle();
        mvpBufferDeviceMemory = static_cast<vulkan::VulkanBufferObjectResource*>(uboResource->getGpuResourceProxy())->getDeviceMemoryHandle();

        ShaderAttachmentInfo uboAttachInfo;
        ShaderAttachmentInfo diffuseTexInfo;
        //UBO Resource
        uboAttachInfo.set = 0;
        uboAttachInfo.binding = 0;
        uboAttachInfo.count = 1;
        uboAttachInfo.shaderStage = EShaderStage::VERTEX;
        uboAttachInfo.resourceType = EShaderResourceType::UNIFORM_BUFFER;
        uboAttachInfo.resource = uboResource;

        // Diffuse Texture resource
        diffuseTexInfo.set = 0;
        diffuseTexInfo.binding = 1;
        diffuseTexInfo.count = 1;
        diffuseTexInfo.shaderStage = EShaderStage::FRAGMENT;
        diffuseTexInfo.resourceType = EShaderResourceType::COMBINED_IMAGE_SAMPLER;
        diffuseTexInfo.resource = texResource;

        GraphicsResource* shaderGroupResource = 
        sgBuilder
        .setVertShaderPath("D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv")
        .setFragShaderPath("D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv")
        .addAttchmentRef(uboAttachInfo)
        .addAttchmentRef(diffuseTexInfo)
        .build();
        
        GeometryRenderInfo renderInfo;
        renderInfo.drawType = EPrimitiveDrawType::INDEXED;
        renderInfo.vertBufferResource = vertResource;
        renderInfo.indexBufferResource = indexResource;
        renderInfo.shaderGroupResource = shaderGroupResource;
        

        vkMapMemory(VulkanRenderer::get()->getDevice(), mvpBufferDeviceMemory, 0, sizeof(UniformBufferObject), 0, &mappedMVPBuffer);

        m_impl = RenderModule::get().getBoltRenderer()->getRenderObjectBuilder()->buildRenderObjectImplFor(this);
        m_impl->build(renderInfo);

        //TODO: Access Vulkan Renderer to create backing images and mapping memory.
        //TODO: use proper interface once available

    }

    void RenderQuad::draw()
    {

    }

    void RenderQuad::drawTemp(vulkan::RenderFrameScene* scene)
    {
        //TODO: Completely refactor this out
        const UniformBufferObject& obj = VulkanRenderer::get()->getMVPMatrix();
        memcpy(mappedMVPBuffer, &obj, sizeof(UniformBufferObject));

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(scene->getCurrentFrameInfo().commandBuffer,
        0, 1, &vertexBuffer, offsets);
        vkCmdBindIndexBuffer(scene->getCurrentFrameInfo().commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        
        m_impl->draw();
        
        vkCmdDrawIndexed(scene->getCurrentFrameInfo().commandBuffer, m_indices.getSize(), 1, 0, 0, 0);
    }

//==========================================================================================
    
    //TODO: Remove these
    VulkanTexture* getTexture_Temp()
    {
        return &texture;
    }
    VkBuffer getUBOBuffer_Temp()
    {
        return mvpBuffer;
    }

}