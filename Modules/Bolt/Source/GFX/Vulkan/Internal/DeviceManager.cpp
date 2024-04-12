#include <GFX/Vulkan/Internal/DeviceManager.h>

DEFINE_SINGLETON(Bolt::DeviceManager)
namespace Bolt
{
    static const DeviceDefinition S_INVALID_DEVICE_DEFINITION = DeviceDefinition();
    static const QueueDefinition S_INVALID_QUEUE_DEFINITION = QueueDefinition();

//============================ QueueDefinition ===============================

    bool QueueDefinition::isvalid() const
    {
        return m_queueFamily != -1 && m_queueFlags != EQueueType::Invalid;
    }
    bool QueueDefinition::supportsGraphicsOps() const
    {
        return m_queueFlags & VK_QUEUE_GRAPHICS_BIT;
    }
    bool QueueDefinition::supportsComputeOps() const
    {
        return m_queueFlags & VK_QUEUE_COMPUTE_BIT;
    }
    bool QueueDefinition::supportsTransferOps() const
    {
        return m_queueFlags & VK_QUEUE_TRANSFER_BIT;
    }
    uint32_t QueueDefinition::getQueueFamily() const
    {
        return m_queueFamily;
    }


//============================================================================

//============================ DeviceDefinition ===============================

    DeviceDefinition::DeviceDefinition(Quaint::IMemoryContext* context)
    : m_queueDefinitions(context)
    {}

    const QueueDefinition& DeviceDefinition::getQueueOfType(const EQueueType flags)
    {
        for(const QueueDefinition& def : m_queueDefinitions)
        {
            if(def.getTypeFlags() == flags)
            {
                return def;
            }
        }

        return S_INVALID_QUEUE_DEFINITION;
    }

//=============================================================================

//============================ DeviceManager ==================================

    DeviceManager::DeviceManager()
    : m_deviceDefinition(Singleton<DeviceManager>::GetContext())
    {
    }

    VkResult DeviceManager::createDevicesAndQueues(const PhysicalDeviceRequirements& phyDevReq, const LogicalDeviceRequirements& logDevReq)
    {
        assert(m_dependencies.instance != VK_NULL_HANDLE);
        return VK_ERROR_UNKNOWN;
    }

    VkPhysicalDevice DeviceManager::getBestPhysicalDeviceFor(VkInstance instance, const PhysicalDeviceRequirements& phyDevReq)
    {
        assert(m_dependencies.instance != VK_NULL_HANDLE && m_dependencies.surface != VK_NULL_HANDLE);

        uint32_t numDevices = 0;
        vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
        assert(numDevices > 0 && "Vulkan could not retrieve any physical devices");

        Quaint::QArray<VkPhysicalDevice> physicalDevices(m_context, numDevices);
        vkEnumeratePhysicalDevices(instance, &numDevices, physicalDevices.getBuffer_NonConst());

        for(const VkPhysicalDevice& device : physicalDevices)
        {
            //size_t deviceScore = getDeviceSuitabilityScore(m_context, device, m_surface);
            //if(deviceScore && deviceScore > score)
            //{
            //    score = deviceScore;
            //    m_physicalDevice = device;
            //}
        }
    }

    uint32_t DeviceManager::getDeviceSuitabilityScore(VkPhysicalDevice device)
    {

    }

//=============================================================================

}