#include <GFX/Vulkan/Internal/VulkanGraphicsContext.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <LoggerModule.h>

DECLARE_LOG_CATEGORY(VulkanGraphicsContext);
DEFINE_LOG_CATEGORY(VulkanGraphicsContext);

namespace Bolt { namespace vulkan
{

    CommandPool::CommandPool(VkCommandPool pPool, const VkCommandPoolCreateFlags pFlags, const QueueDefinition& pQueueDef)
    : commandPool(pPool)
    , flags(pFlags)
    , queueDef(pQueueDef)
    {}

    GraphicsContext::GraphicsContext(Quaint::IMemoryContext* context)
    : m_context(context)
    {}

    void GraphicsContext::construct()
    {
        m_valid = true;
    }

    void GraphicsContext::destroy()
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        if(m_commandPool.commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, m_commandPool.commandPool, callbacks);
            m_commandPool.commandPool = VK_NULL_HANDLE;
        }
    }

    CommandPool& GraphicsContext::buildCommandPool(const VkCommandPoolCreateFlags flags, const EQueueTypeFlags supportedQueues)
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        const QueueDefinition& def = dm->getDeviceDefinition().getQueueOfType(supportedQueues);
        assert(def.isvalid() && "could not find a queue with required support");
        
        VkCommandPoolCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = flags;
        info.queueFamilyIndex = def.getQueueFamily();

        VkCommandPool pool = VK_NULL_HANDLE;
        ASSERT_SUCCESS(
            vkCreateCommandPool(dm->getDeviceDefinition().getDevice(), &info, callbacks, &pool)
            , "Failed to create command pool"
        );
        
        m_commandPool = CommandPool(pool, flags, def);
        return m_commandPool;
    }

    void GraphicsContext::addCommandBuffer(const VkCommandBufferLevel level, const uint32_t count, VkCommandBuffer* buffers)
    {
        assert(buffers != nullptr && "Need valid pointer to to the allocated command buffer");

        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.commandBufferCount = count;
        info.level = level;

        Quaint::QArray<VkCommandBuffer> allocatedBuffers(m_context, count);
        ASSERT_SUCCESS(vkAllocateCommandBuffers(device, &info, allocatedBuffers.getBuffer_NonConst()),
                        "Failed to allocate all the required command buffers");
    }

    void GraphicsContext::removeCommandBuffer(const VkCommandBuffer* buffers, const uint32_t count)
    {
        if(buffers != nullptr && count > 0)
        {
            QLOG_E(VulkanGraphicsContext, "Cannot remove command buffer. Invalid data provided.");
            return;
        }
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        vkFreeCommandBuffers(device, m_commandPool.commandPool, count, buffers);
    }
}}