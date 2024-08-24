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

    void GraphicsContext::buildCommandPool(const VkCommandPoolCreateFlags flags, const EQueueTypeFlags supportedQueues)
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        const QueueDefinition& def = dm->getDeviceDefinition().getQueueOfType(supportedQueues);
        if (!def.isvalid())
        {
            QLOG_E(VulkanGraphicsContext, "could not find a queue with required support");
            return;
        }
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
    }

}}