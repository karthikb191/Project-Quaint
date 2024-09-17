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
        ShaderInfo info{};
        info.vertShaderPath = "D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv";
        info.fragShaderPath = "D:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv";
        info.resources = Quaint::QArray<ShaderResource>(m_context);

        // TODO: Create a different resource type inheriting from this 
        ShaderResource resource;
        //UBO Resource
        resource.set = 0;
        resource.binding = 0;
        resource.count = 1;
        resource.perFrame = false;
        resource.stage = EShaderStage::VERTEX;
        resource.type = EResourceType::UNIFORM_BUFFER;
        info.resources.pushBack(resource);
 
        // Diffuse Texture resource
        resource.set = 0;
        resource.binding = 1;
        resource.count = 1;
        resource.perFrame = false;
        resource.stage = EShaderStage::FRAGMENT;
        resource.type = EResourceType::COMBINED_IMAGE_SAMPLER;
        info.resources.pushBack(resource);
        info.maxResourceSets = 1;

        // Get Backed Texture
        // Get Backed Buffer
        VulkanRenderer::get()->createTextureFromFile("D:\\Works\\Project-Quaint\\Data\\Textures\\Test\\test.jpg", texture);

        constexpr int sz = sizeof(Quaint::QVertex);
        VulkanRenderer::get()->createBuffer(
            sizeof(decltype(m_vertices)::value_type) * sizeof(QVertex)
            , (void*)m_vertices.getBuffer()
            , VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            , vertexBufferDeviceMemory
            , vertexBuffer
        );

        VulkanRenderer::get()->createBuffer(
            sizeof(decltype(m_indices)::value_type) * m_indices.getSize()
            , (void*)m_indices.getBuffer()
            , VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            , indexBufferDeviceMemory
            , indexBuffer
        );

        VulkanRenderer::get()->createBuffer(
            sizeof(UniformBufferObject)
            , (void*)m_indices.getBuffer()
            , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
            , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , mvpBufferDeviceMemory
            , mvpBuffer
        );
        vkMapMemory(VulkanRenderer::get()->getDevice(), mvpBufferDeviceMemory, 0, sizeof(UniformBufferObject), 0, &mappedMVPBuffer);

        m_impl = RenderModule::get().getBoltRenderer()->getRenderObjectBuilder()->buildRenderObjectImplFor(this);
        m_impl->build(info);

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