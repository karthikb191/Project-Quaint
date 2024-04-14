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
        m_deviceDefinition.m_physicalDevice = getBestPhysicalDeviceFor(phyDevReq);
        assert(m_deviceDefinition.m_physicalDevice != VK_NULL_HANDLE && "Could not find appropriate physical device for given requirements");

        return VK_SUCCESS;
    }

    VkPhysicalDevice DeviceManager::getBestPhysicalDeviceFor(const PhysicalDeviceRequirements& phyDevReq)
    {
        assert(m_dependencies.instance != VK_NULL_HANDLE && m_dependencies.surface != VK_NULL_HANDLE);
        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

        uint32_t numDevices = 0;
        vkEnumeratePhysicalDevices(m_dependencies.instance, &numDevices, nullptr);
        assert(numDevices > 0 && "Vulkan could not retrieve any physical devices");

        Quaint::QArray<VkPhysicalDevice> physicalDevices(m_context, numDevices);
        vkEnumeratePhysicalDevices(m_dependencies.instance, &numDevices, physicalDevices.getBuffer_NonConst());

        uint32_t score = 0;
        for(const VkPhysicalDevice& device : physicalDevices)
        {
            size_t deviceScore = getDeviceSuitabilityScore(device, phyDevReq);
            if(deviceScore > score)
            {
                score = deviceScore;
                bestDevice = device;
            }
        }

        return bestDevice;
    }

    uint32_t DeviceManager::getDeviceSuitabilityScore(VkPhysicalDevice device, const PhysicalDeviceRequirements& phyDevReq)
    {
        uint32_t score = 0;
        VkPhysicalDeviceProperties2 properties;
        VkPhysicalDeviceFeatures2 features;

        vkGetPhysicalDeviceProperties2(device, &properties);
        vkGetPhysicalDeviceFeatures2(device, &features);
 
        uint32_t extensionsCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
        Quaint::QArray<VkExtensionProperties> extensionProperties(m_context, extensionsCount);
        VkResult res = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.getBuffer_NonConst());

        assert(res == VK_SUCCESS && "Could not enumerate device extension properties");

        for(const Quaint::QString256& extension : phyDevReq.extensions)
        {
            bool found = false;
            for(const VkExtensionProperties& property : extensionProperties)
            {
                if(strcmp(property.extensionName, extension.getBuffer()) == 0)
                {
                    found = true;
                    break;
                }
            }

            //Required extensions are not found in the returned enumeration
            if(!found)
            {
                return 0;
            }
        }

        //Checking for the required queue support
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        Quaint::QArray<VkQueueFamilyProperties> queueFamilyProps(m_context, queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProps.getBuffer_NonConst());

        uint32_t reqOpMask = 0;
        reqOpMask |= phyDevReq.graphics.numRequired > 0 ? VK_QUEUE_GRAPHICS_BIT : 0;
        reqOpMask |= phyDevReq.compute.numRequired > 0 ? VK_QUEUE_COMPUTE_BIT : 0;
        reqOpMask |= phyDevReq.transfer.numRequired > 0 ? VK_QUEUE_TRANSFER_BIT : 0;

        //Broad check to see if a device has suitable queues
        uint32_t foundOpMask = 0;
        for(const VkQueueFamilyProperties& property : queueFamilyProps)
        {
            if(phyDevReq.strictlyIdealQueueFamilyRequired)
            {
                if(property.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))
                {
                    foundOpMask = property.queueFlags;
                }
            }
            else
            {
                if(phyDevReq.graphics.numRequired > 0 && (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                || phyDevReq.compute.numRequired > 0 && (property.queueFlags & VK_QUEUE_COMPUTE_BIT)
                || phyDevReq.transfer.numRequired > 0 && (property.queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    foundOpMask |= property.queueFlags;
                }
            }
            if((foundOpMask & reqOpMask) == reqOpMask)
            {
                break;
            }
        }

        // Not all required queues were found on the device.....
        if((foundOpMask & reqOpMask) != reqOpMask)
        {
            return 0;
        }


        //TODO: Query swapchain support and required surface capabilities
        //----------------------------------------


        //Additional nice to haves
        if(features.features.tessellationShader) score += 100;
        if(features.features.imageCubeArray) score += 100;
        if(properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100;

        return score;
    }

    void DeviceManager::populateQueueDefinition(const PhysicalDeviceRequirements& phyDevReq)
    {
        assert(m_deviceDefinition.m_physicalDevice != VK_NULL_HANDLE && "Trying to fetch queue for invalid handle");

        VkPhysicalDevice device = m_deviceDefinition.m_physicalDevice;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        Quaint::QArray<VkQueueFamilyProperties> queueFamilyProps(m_context, queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProps.getBuffer_NonConst());

        uint32_t reqOpMask = 0;
        reqOpMask |= phyDevReq.graphics.numRequired > 0 ? VK_QUEUE_GRAPHICS_BIT : 0;
        reqOpMask |= phyDevReq.compute.numRequired > 0 ? VK_QUEUE_COMPUTE_BIT : 0;
        reqOpMask |= phyDevReq.transfer.numRequired > 0 ? VK_QUEUE_TRANSFER_BIT : 0;
        m_deviceDefinition.m_queueDefinitions = Quaint::QArray<QueueDefinition>(m_context);
        
        uint32_t numGraphicsRequired = phyDevReq.graphics.numRequired;
        uint32_t numComputeRequired = phyDevReq.compute.numRequired;
        uint32_t numTransferRequired = phyDevReq.transfer.numRequired;
        uint32_t foundOpMask = 0;
        for(size_t i = 0; i < queueFamilyCount; ++i)
        {
            const VkQueueFamilyProperties& property = queueFamilyProps[i];
            QueueDefinition queueDefinition;
            
            uint32_t currentOpMask = 0;

            if(numGraphicsRequired > 0
            && property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                --numGraphicsRequired;
                currentOpMask |= VK_QUEUE_GRAPHICS_BIT;
            }
            if(numComputeRequired > 0
            && property.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                --numComputeRequired;
                currentOpMask |= VK_QUEUE_COMPUTE_BIT;
            }
            if(numTransferRequired > 0
            && property.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                --numTransferRequired;
                currentOpMask |= VK_QUEUE_TRANSFER_BIT;
            }

            currentOpMask |= foundOpMask;

            //Queues in current queue family supports all the required operations
            if(currentOpMask != 0)
            {
                queueDefinition.m_queueFamily = i;
                queueDefinition.m_queueFlags = currentOpMask; //Only register the ops that we are supporting.

                bool registerQueueDefinition = false;
                if(phyDevReq.strictlyIdealQueueFamilyRequired)
                {
                    if((currentOpMask & reqOpMask) == reqOpMask)
                    {
                        registerQueueDefinition = true;
                    }
                }
                else
                {
                    registerQueueDefinition = true;
                }

                if(registerQueueDefinition)
                {
                    m_deviceDefinition.m_queueDefinitions.pushBack(queueDefinition);
                }
            }

            //All queues found. Break out
            if((foundOpMask & reqOpMask) == reqOpMask)
            {
                break;
            }
        }

        assert( (foundOpMask & reqOpMask) == reqOpMask && "All Queues not found");
    }

    VkDevice DeviceManager::createLogicalDevice(const LogicalDeviceRequirements& logDevReq)
    {
        return VK_NULL_HANDLE;
    }

//=============================================================================

}