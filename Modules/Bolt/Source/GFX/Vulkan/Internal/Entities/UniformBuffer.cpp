
#include <GFX/Vulkan/Internal/Entities/UniformBuffer.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt{ namespace vulkan {
    
    UniformBuffer::UniformBuffer(Quaint::IMemoryContext* context, uint32_t size)
    : m_context(context)
    , m_size(size)
    {
        VulkanRenderer::get()->createBuffer(size, 
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_gpuMemory, m_buffer);

    }
    UniformBuffer::UniformBuffer(Quaint::IMemoryContext* context, void* data, uint32_t size)
    : m_context(context)
    , m_size(size)
    {
        VulkanRenderer::get()->createBuffer(size, data, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_gpuMemory, m_buffer);

        // Map memory and copy initial data contents.
    }

    void UniformBuffer::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        vkFreeMemory(device, m_gpuMemory, callbacks);
        vkDestroyBuffer(device, m_buffer, callbacks);
    }

    void* UniformBuffer::map()
    {
        assert(m_gpuMemory != VK_NULL_HANDLE && m_buffer != VK_NULL_HANDLE && "buffer not created or no gpu memory available");
        VkDevice device = VulkanRenderer::get()->getDevice();

        void** mappedMemory = nullptr;
        vkMapMemory(device, m_gpuMemory, 0, m_size, 0, mappedMemory);
        return mappedMemory;
    }
    void UniformBuffer::unmap()
    {
        assert(m_gpuMemory != VK_NULL_HANDLE && m_buffer != VK_NULL_HANDLE && "buffer not created or no gpu memory available");
        VkDevice device = VulkanRenderer::get()->getDevice();

        vkUnmapMemory(device, m_gpuMemory);
    }

}}